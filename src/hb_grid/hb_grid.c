#include "hb_grid.h"

#include "hb_log/hb_log.h"
#include "hb_log_tcp/hb_log_tcp.h"
#include "hb_log_file/hb_log_file.h"
#include "hb_cache/hb_cache.h"
#include "hb_utils/hb_memmem.h"
#include "hb_utils/hb_multipart.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_date.h"

//////////////////////////////////////////////////////////////////////////
static void __hb_log_observer( const char * _category, hb_log_level_e _level, const char * _message )
{
    const char * ls = hb_log_level_string[_level];

    printf( "[%s] %s: %s\n", _category, ls, _message );
}
//////////////////////////////////////////////////////////////////////////
extern int hb_grid_request_api( struct evhttp_request * _request, struct hb_grid_process_handle_t * _handle, char * _response, size_t * _size );
extern int hb_grid_request_upload( struct evhttp_request * _request, struct hb_grid_process_handle_t * _handle, char * _response, size_t * _size );
extern int hb_grid_request_newproject( struct evhttp_request * _request, struct hb_grid_process_handle_t * _handle, char * _response, size_t * _size );
extern int hb_grid_request_newuser( struct evhttp_request * _request, struct hb_grid_process_handle_t * _handle, char * _response, size_t * _size );
extern int hb_grid_request_loginuser( struct evhttp_request * _request, struct hb_grid_process_handle_t * _handle, char * _response, size_t * _size );
////////////////////////////////////////////////////////////////////////
static void __hb_grid_request( struct evhttp_request * _request, void * _ud )
{
    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );

    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    enum evhttp_cmd_type command_type = evhttp_request_get_command( _request );
    
    if( command_type == EVHTTP_REQ_OPTIONS )
    {
        struct evkeyvalq * output_headers = evhttp_request_get_output_headers( _request );

        evhttp_add_header( output_headers, "Access-Control-Allow-Origin", "*" );
        evhttp_add_header( output_headers, "Access-Control-Allow-Headers", "*" );
        evhttp_add_header( output_headers, "Access-Control-Allow-Methods", "POST" );
        evhttp_add_header( output_headers, "Content-Type", "application/json" );

        evhttp_send_reply( _request, HTTP_OK, "", output_buffer );

        return;
    }

    hb_grid_request_handle_t * handle = (hb_grid_request_handle_t *)_ud;

    size_t response_data_size = 2;
    char response_data[HB_GRID_REQUEST_DATA_MAX_SIZE];

    strcpy( response_data, "{}" );

    int32_t code = (*handle->request)(_request, handle->process, response_data, &response_data_size);

    evbuffer_add( output_buffer, response_data, response_data_size );

    struct evkeyvalq * output_headers = evhttp_request_get_output_headers( _request );

    evhttp_add_header( output_headers, "Access-Control-Allow-Origin", "*" );
    evhttp_add_header( output_headers, "Access-Control-Allow-Headers", "*" );
    evhttp_add_header( output_headers, "Access-Control-Allow-Methods", "POST" );
    evhttp_add_header( output_headers, "Content-Type", "application/json" );

    evhttp_send_reply( _request, code, "", output_buffer );
}
//////////////////////////////////////////////////////////////////////////
static uint32_t __stdcall __hb_ev_thread_base( void * _ud )
{
    HB_UNUSED( _ud );

    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

    struct event_base * base = event_base_new();
    HB_UNUSED( base );

    struct evhttp * http_server = evhttp_new( base );
    HB_UNUSED( http_server );

    evhttp_set_allowed_methods( http_server, EVHTTP_REQ_POST | EVHTTP_REQ_OPTIONS );

    handle->requests[0].request = &hb_grid_request_api;
    handle->requests[0].process = handle;

    handle->requests[1].request = &hb_grid_request_upload;
    handle->requests[1].process = handle;

    handle->requests[2].request = &hb_grid_request_newproject;
    handle->requests[2].process = handle;

    handle->requests[3].request = &hb_grid_request_newuser;
    handle->requests[3].process = handle;

    handle->requests[4].request = &hb_grid_request_loginuser;
    handle->requests[4].process = handle;

    evhttp_set_cb( http_server, "/api", &__hb_grid_request, handle->requests + 0 );
    evhttp_set_cb( http_server, "/upload", &__hb_grid_request, handle->requests + 1 );
    evhttp_set_cb( http_server, "/newproject", &__hb_grid_request, handle->requests + 2 );
    evhttp_set_cb( http_server, "/newuser", &__hb_grid_request, handle->requests + 3 );
    evhttp_set_cb( http_server, "/loginuser", &__hb_grid_request, handle->requests + 4 );

    if( *handle->ev_socket == -1 )
    {
        struct evhttp_bound_socket * bound_socket = evhttp_bind_socket_with_handle( http_server, handle->server_address, handle->server_port );
        HB_UNUSED( bound_socket );

        *handle->ev_socket = evhttp_bound_socket_get_fd( bound_socket );
    }
    else
    {
        evhttp_accept_socket( http_server, *handle->ev_socket );
    }

    event_base_dispatch( base );
    
    evhttp_free( http_server );

    return 0;
}
//////////////////////////////////////////////////////////////////////////
int main( int _argc, char * _argv[] )
{
    HB_UNUSED( _argc );
    HB_UNUSED( _argv );

    hb_log_initialize();

    if( hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_observer ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    const char * id;
    if( hb_getopt( _argc, _argv, "--id", &id ) == HB_FAILURE )
    {
        hb_log_message( "grid", HB_LOG_CRITICAL, "run without id [miss --id argument]" );

        return EXIT_FAILURE;
    }    

    const uint32_t max_thread = 16;

#ifdef WIN32
    const WORD wVersionRequested = MAKEWORD( 2, 2 );

    WSADATA wsaData;
    int err = WSAStartup( wVersionRequested, &wsaData );

    if( err != 0 )
    {
        return EXIT_FAILURE;
    }
#endif

    if( hb_log_tcp_initialize( "127.0.0.1", 5044 ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_date_t date;
    hb_date( &date );

    char logfile[HB_MAX_PATH];
    sprintf( logfile, "hb_grid_log_%u_%u_%u_%u_%u_%u.log"
        , date.year
        , date.mon
        , date.mday
        , date.hour
        , date.min
        , date.sec );

    if( hb_log_file_initialize( logfile ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_grid_process_handle_t * process_handles = HB_NEWN( hb_grid_process_handle_t, max_thread );

    evutil_socket_t ev_socket = -1;
    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_grid_process_handle_t * process_handle = process_handles + i;

        if( hb_sharedmemory_create( i, 65536, &process_handle->sharedmemory ) == HB_FAILURE )
        {
            continue;
        }

        strcpy( process_handle->server_address, "127.0.0.1" );
        process_handle->server_port = 5555;

        process_handle->ev_socket = &ev_socket;

        strcpy( process_handle->config.db_uri, "127.0.0.1" );
        process_handle->config.db_port = 27017;

        strcpy( process_handle->config.cache_uri, "127.0.0.1" );
        process_handle->config.cache_port = 6379;

        sprintf( process_handle->config.log_file, "hb_%s_log_%03u_%u_%u_%u_%u_%u_%u.log"
            , id
            , i
            , date.year
            , date.mon
            , date.mday
            , date.hour
            , date.min
            , date.sec );

        strcpy( process_handle->config.log_uri, "127.0.0.1" );        
        process_handle->config.log_port = 5044;

        if( hb_thread_create( &__hb_ev_thread_base, process_handle, &process_handle->thread ) == HB_FAILURE )
        {
            continue;
        }

        Sleep( 100 ); //hack
    }

    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_grid_process_handle_t * process_handle = process_handles + i;

        hb_thread_join( process_handle->thread );
    }

    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_grid_process_handle_t * process_handle = process_handles + i;

        hb_thread_destroy( process_handle->thread );
        hb_sharedmemory_destroy( process_handle->sharedmemory );
    }

    HB_DELETE( process_handles );

#ifdef WIN32
    WSACleanup();
#endif

    hb_log_finalize();

    return EXIT_SUCCESS;
}