#include "hb_grid.h"

#include "hb_log/hb_log.h"
#include "hb_log_tcp/hb_log_tcp.h"
#include "hb_log_file/hb_log_file.h"
#include "hb_json/hb_json.h"
#include "hb_cache/hb_cache.h"
#include "hb_utils/hb_memmem.h"
#include "hb_utils/hb_multipart.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_date.h"

//////////////////////////////////////////////////////////////////////////
static void __hb_log_observer( const char * _category, hb_log_level_t _level, const char * _file, uint32_t _line, const char * _message, void * _ud )
{
    HB_UNUSED( _ud );

    const char * ls = hb_log_level_string[_level];

    printf( "%s [%s:%u] %s: %s\n", ls, _file, _line, _category, _message );
}
//////////////////////////////////////////////////////////////////////////
extern int hb_grid_request_newproject( struct evhttp_request * _request, struct hb_grid_process_handle_t * _handle, char * _response, size_t * _size );
extern int hb_grid_request_api( struct evhttp_request * _request, struct hb_grid_process_handle_t * _handle, char * _response, size_t * _size, const char * _pid, const char * _token, const char * _method );
extern int hb_grid_request_upload( struct evhttp_request * _request, struct hb_grid_process_handle_t * _handle, char * _response, size_t * _size, const char * _pid );
extern int hb_grid_request_newuser( struct evhttp_request * _request, struct hb_grid_process_handle_t * _handle, char * _response, size_t * _size, const char * _pid );
extern int hb_grid_request_loginuser( struct evhttp_request * _request, struct hb_grid_process_handle_t * _handle, char * _response, size_t * _size, const char * _pid );
////////////////////////////////////////////////////////////////////////
static void __hb_grid_request( struct evhttp_request * _request, void * _ud )
{
    const char * host = evhttp_request_get_host( _request );
    HB_UNUSED( host );

    const char * uri = evhttp_request_get_uri( _request );
    HB_UNUSED( uri );

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

    hb_grid_process_handle_t * process = (hb_grid_process_handle_t *)_ud;

    int32_t response_code = HTTP_OK;

    size_t response_data_size = 2;
    char response_data[HB_GRID_REQUEST_DATA_MAX_SIZE];
    strcpy( response_data, "{}" );

    char pid[128] = { '\0' };

    if( strcmp( uri, "/newproject" ) == 0 )
    {
        response_code = hb_grid_request_newproject( _request, process, response_data, &response_data_size );
    }
    else
    {
        char cmd[128] = { '\0' };
        char token[128] = { '\0' };
        char method[128] = { '\0' };
        int count = sscanf( uri, "/%[^'/']/%[^'/']/%[^'/']/%[^'/']", pid, cmd, token, method );

        if( count < 2 )
        {
            evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

            return;
        }

        if( strcmp( cmd, "api" ) == 0 )
        {
            if( count < 4 )
            {
                evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

                return;
            }

            response_code = hb_grid_request_api( _request, process, response_data, &response_data_size, pid, token, method );
        }
        else if( strcmp( cmd, "upload" ) == 0 )
        {
            response_code = hb_grid_request_upload( _request, process, response_data, &response_data_size, pid );
        }
        else if( strcmp( cmd, "newuser" ) == 0 )
        {
            response_code = hb_grid_request_newuser( _request, process, response_data, &response_data_size, pid );
        }
        else if( strcmp( cmd, "loginuser" ) == 0 )
        {
            response_code = hb_grid_request_loginuser( _request, process, response_data, &response_data_size, pid );
        }
    }

    evbuffer_add( output_buffer, response_data, response_data_size );

    struct evkeyvalq * output_headers = evhttp_request_get_output_headers( _request );

    evhttp_add_header( output_headers, "Access-Control-Allow-Origin", "*" );
    evhttp_add_header( output_headers, "Access-Control-Allow-Headers", "*" );
    evhttp_add_header( output_headers, "Access-Control-Allow-Methods", "POST" );
    evhttp_add_header( output_headers, "Content-Type", "application/json" );

    evhttp_send_reply( _request, response_code, "", output_buffer );
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

    evhttp_set_gencb( http_server, &__hb_grid_request, handle );

    if( *handle->ev_socket == -1 )
    {
        struct evhttp_bound_socket * bound_socket = evhttp_bind_socket_with_handle( http_server, handle->grid_uri, handle->grid_port );
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

    if( hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_observer, HB_NULLPTR ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

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

#ifdef HB_DEBUG
    hb_date_t date;
    hb_date( &date );

    char logfile[HB_MAX_PATH];
    sprintf( logfile, "log_grid_%u_%u_%u_%u_%u_%u.log"
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
#endif

    const char * config_file = HB_NULLPTR;
    hb_getopt( _argc, _argv, "--config", &config_file );

    hb_node_config_t * config = HB_NEW( hb_node_config_t );

    uint32_t max_thread = 16;

    char grid_uri[128];
    strcpy( grid_uri, "127.0.0.1" );
    uint16_t grid_port = 5555;

    strcpy( config->name, "hb" );
    strcpy( config->cache_uri, "127.0.0.1" );
    config->cache_port = 6379;
    config->cache_timeout = 2000;
    strcpy( config->db_uri, "127.0.0.1" );
    config->db_port = 27017;
    strcpy( config->log_uri, "127.0.0.1" );
    config->log_port = 5044;

    if( config_file != HB_NULLPTR )
    {
        FILE * f = fopen( config_file, "rb" );

        if( f == HB_NULLPTR )
        {
            HB_LOG_MESSAGE_CRITICAL( "grid", "config file '%s' not found"
                , config_file
            );

            return EXIT_FAILURE;
        }

        fseek( f, 0L, SEEK_END );
        long sz = ftell( f );
        rewind( f );

        if( sz > 10240 )
        {
            HB_LOG_MESSAGE_CRITICAL( "grid", "config file '%s' very large [%d]"
                , config_file
                , sz
            );

            return EXIT_FAILURE;
        }

        char config_buffer[10240];
        size_t r = fread( config_buffer, sz, 1, f );
        HB_UNUSED( r );

        fclose( f );

        hb_json_handle_t * json_handle;
        if( hb_json_create( config_buffer, sz, &json_handle ) == HB_FAILURE )
        {
            HB_LOG_MESSAGE_CRITICAL( "grid", "config file '%s' wrong json"
                , config_file
            );

            return EXIT_FAILURE;
        }

        int64_t config_max_thread = 0;
        hb_json_get_field_integer( json_handle, "max_thread", &config_max_thread, max_thread );
        max_thread = (uint32_t)config_max_thread;

        const char * config_grid_uri = HB_NULLPTR;
        hb_json_get_field_string( json_handle, "grid_uri", &config_grid_uri, HB_NULLPTR, grid_uri );
        strcpy( grid_uri, config_grid_uri );

        int64_t config_grid_port = 0;
        hb_json_get_field_integer( json_handle, "grid_port", &config_grid_port, grid_port );
        grid_port = (uint16_t)config_grid_port;

        const char * name = HB_NULLPTR;
        hb_json_get_field_string( json_handle, "name", &name, HB_NULLPTR, config->name );
        strcpy( config->name, name );

        const char * cache_uri = HB_NULLPTR;
        hb_json_get_field_string( json_handle, "cache_uri", &cache_uri, HB_NULLPTR, config->cache_uri );
        strcpy( config->cache_uri, cache_uri );

        int64_t cache_port = 0;
        hb_json_get_field_integer( json_handle, "cache_port", &cache_port, config->cache_port );
        config->cache_port = (uint16_t)cache_port;

        int64_t cache_timeout = 0;
        hb_json_get_field_integer( json_handle, "cache_timeout", &cache_timeout, config->cache_timeout );
        config->cache_timeout = (uint16_t)cache_timeout;

        const char * db_uri = HB_NULLPTR;
        hb_json_get_field_string( json_handle, "db_uri", &db_uri, HB_NULLPTR, config->db_uri );
        strcpy( config->db_uri, db_uri );

        int64_t db_port = 0;
        hb_json_get_field_integer( json_handle, "db_port", &db_port, config->db_port );
        config->db_port = (uint16_t)db_port;

        const char * log_uri = HB_NULLPTR;
        hb_json_get_field_string( json_handle, "log_uri", &log_uri, HB_NULLPTR, config->log_uri );
        strcpy( config->log_uri, log_uri );

        int64_t log_port = 0;
        hb_json_get_field_integer( json_handle, "log_port", &log_port, config->log_port );
        config->log_port = (uint16_t)log_port;

        hb_json_destroy( json_handle );
    }

    HB_LOG_MESSAGE_INFO( "grid", "start grid with config:" );
    HB_LOG_MESSAGE_INFO( "grid", "max_thread: %u", max_thread );
    HB_LOG_MESSAGE_INFO( "grid", "grid_uri: %s", grid_uri );
    HB_LOG_MESSAGE_INFO( "grid", "grid_port: %u", grid_port );
    HB_LOG_MESSAGE_INFO( "grid", "name: %s", config->name );
    HB_LOG_MESSAGE_INFO( "grid", "cache_uri: %s", config->cache_uri );
    HB_LOG_MESSAGE_INFO( "grid", "cache_port: %u", config->cache_port );
    HB_LOG_MESSAGE_INFO( "grid", "cache_timeout: %u", config->cache_timeout );
    HB_LOG_MESSAGE_INFO( "grid", "db_uri: %s", config->db_uri );
    HB_LOG_MESSAGE_INFO( "grid", "db_port: %u", config->db_port );
    HB_LOG_MESSAGE_INFO( "grid", "log_uri: %s", config->log_uri );
    HB_LOG_MESSAGE_INFO( "grid", "log_port: %u", config->log_port );

    hb_grid_process_handle_t * process_handles = HB_NEWN( hb_grid_process_handle_t, max_thread );

    evutil_socket_t ev_socket = -1;
    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_grid_process_handle_t * process_handle = process_handles + i;

        if( hb_sharedmemory_create( i, 65536, &process_handle->sharedmemory ) == HB_FAILURE )
        {
            continue;
        }

        strcpy( process_handle->grid_uri, grid_uri );
        process_handle->grid_port = grid_port;

        process_handle->ev_socket = &ev_socket;

        process_handle->config = config;

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

    HB_DELETE( config );

#ifdef WIN32
    WSACleanup();
#endif

    hb_log_remove_observer( &__hb_log_observer, HB_NULLPTR );

#ifdef HB_DEBUG
    hb_log_file_finalize();
#endif

    hb_log_finalize();

    return EXIT_SUCCESS;
}