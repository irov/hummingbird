#include "hb_grid.h"

#include "hb_log/hb_log.h"
#include "hb_utils/hb_memmem.h"
#include "hb_utils/hb_multipart.h"
#include "hb_utils/hb_getopt.h"

//////////////////////////////////////////////////////////////////////////
static void __hb_log_observer( const char * _category, int _level, const char * _message )
{
    const char * ls[] = { "info", "warning", "error", "critical" };

    printf( "[%s] %s: %s\n", _category, ls[_level], _message );
}

//////////////////////////////////////////////////////////////////////////
extern void hb_grid_request_api( struct evhttp_request *, void * );
extern void hb_grid_request_upload( struct evhttp_request *, void * );
extern void hb_grid_request_newproject( struct evhttp_request *, void * );
//////////////////////////////////////////////////////////////////////////
static uint32_t __stdcall __hb_ev_thread_base( void * _ud )
{
    HB_UNUSED( _ud );

    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

    struct event_base * base = event_base_new();
    HB_UNUSED( base );

    struct evhttp * http_server = evhttp_new( base );
    HB_UNUSED( http_server );

    evhttp_set_cb( http_server, "/api", &hb_grid_request_api, handle );
    evhttp_set_cb( http_server, "/upload", &hb_grid_request_upload, handle );
    evhttp_set_cb( http_server, "/newproject", &hb_grid_request_newproject, handle );

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
    hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_observer );

    const char * id;
    if( hb_getopt( _argc, _argv, "--id", &id ) == 0 )
    {
        return EXIT_FAILURE;
    }

    //char const server_address[] = "127.0.0.1";
    //char const server_address[] = "0.0.0.0";
    //ev_uint16_t server_port = 5555;

    const uint32_t max_thread = 16;

    WORD wVersionRequested = MAKEWORD( 2, 2 );

    WSADATA wsaData;
    int err = WSAStartup( wVersionRequested, &wsaData );

    if( err != 0 )
    {
        return EXIT_FAILURE;
    }

    hb_grid_process_handle_t * process_handles = HB_NEWN( hb_grid_process_handle_t, max_thread );

    evutil_socket_t ev_socket = -1;
    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_grid_process_handle_t * process_handle = process_handles + i;

        strcpy( process_handle->server_address, "127.0.0.1" );
        process_handle->server_port = 5555;

        process_handle->ev_socket = &ev_socket;

        hb_thread_create( &__hb_ev_thread_base, process_handle, &process_handle->thread );

        char sharedmemory_name[64];
        sprintf( sharedmemory_name, "hb_%s_sm_%03u", id, i );

        hb_sharedmemory_create( sharedmemory_name, 10240, &process_handle->sharedmemory );

        Sleep( 100 ); //hack
    }

    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_grid_process_handle_t * process_handle = process_handles + i;

        hb_thread_join( &process_handle->thread );
    }

    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_grid_process_handle_t * process_handle = process_handles + i;

        hb_thread_destroy( &process_handle->thread );
        hb_sharedmemory_destroy( &process_handle->sharedmemory );
    }

    HB_DELETE( process_handles );

    WSACleanup();

    hb_log_finalize();

    return EXIT_SUCCESS;
}