#include "hb_grid.h"

#include "hb_sharedmemory/hb_sharedmemory.h"
#include "hb_process/hb_process.h"
#include "hb_log/hb_log.h"

#include "evhttp.h"
#include "event2/thread.h"

#include <process.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_grid_process_handler_t
{
    char server_address[64];
    ev_uint16_t server_port;

    evutil_socket_t * ev_socket;

    uint32_t threadId;
    HANDLE hThread;

    hb_sharedmemory_handler_t sharedmemory;
} hb_grid_process_handler_t;
//////////////////////////////////////////////////////////////////////////
static void __hb_ev_on_request( struct evhttp_request * _request, void * _ud )
{
    HB_UNUSED( _request );
    
    hb_grid_process_handler_t * handler = (hb_grid_process_handler_t *)_ud;

    enum evhttp_cmd_type command_type = evhttp_request_get_command( _request );
    HB_UNUSED( command_type );

    struct evkeyvalq * keyval = evhttp_request_get_input_headers( _request );
    HB_UNUSED( keyval );

    struct evbuffer * input_buffer = evhttp_request_get_input_buffer( _request );
    HB_UNUSED( input_buffer );

    uint8_t copyout_buffer[2048];
    ev_ssize_t copyout_buffer_size = evbuffer_copyout( input_buffer, copyout_buffer, 2048 );
    HB_UNUSED( copyout_buffer_size );

    hb_sharedmemory_write( &handler->sharedmemory, copyout_buffer, copyout_buffer_size );

    char process_command[64];
    sprintf( process_command, "--sm %s"
        , handler->sharedmemory.name
    );

    hb_process_run( "hb_node.exe", process_command );

    hb_sharedmemory_rewind( &handler->sharedmemory );

    size_t process_result_size;
    char process_result[2048];
    hb_sharedmemory_read( &handler->sharedmemory, process_result, 2048, &process_result_size );

    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );
    
    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    evbuffer_add( output_buffer, process_result, process_result_size );

    evhttp_send_reply( _request, HTTP_OK, "", output_buffer );
}
//////////////////////////////////////////////////////////////////////////
static uint32_t __stdcall __hb_ev_thread_base( void * _ud )
{
    HB_UNUSED( _ud );

    hb_grid_process_handler_t * handler = (hb_grid_process_handler_t *)_ud;

    struct event_base * base = event_base_new();
    HB_UNUSED( base );

    struct evhttp * http_server = evhttp_new( base );
    HB_UNUSED( http_server );

    evhttp_set_gencb( http_server, &__hb_ev_on_request, handler );

    if( *handler->ev_socket == -1 )
    {
        struct evhttp_bound_socket * bound_socket = evhttp_bind_socket_with_handle( http_server, handler->server_address, handler->server_port );
        HB_UNUSED( bound_socket );

        *handler->ev_socket = evhttp_bound_socket_get_fd( bound_socket );
    }
    else
    {
        evhttp_accept_socket( http_server, *handler->ev_socket );
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

    hb_grid_process_handler_t * process_handlers = HB_NEWN( hb_grid_process_handler_t, max_thread );

    evutil_socket_t ev_socket = -1;
    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_grid_process_handler_t * process_handler = process_handlers + i;

        strcpy( process_handler->server_address, "127.0.0.1" );
        process_handler->server_port = 5555;

        process_handler->ev_socket = &ev_socket;

        uint32_t threadId;
        HANDLE hThread = (HANDLE)_beginthreadex( HB_NULLPTR, 0, &__hb_ev_thread_base, process_handler, 0, &threadId );
                
        process_handler->hThread = hThread;
        process_handler->threadId = threadId;

        char sharedmemory_name[64];
        sprintf( sharedmemory_name, "hb_sharedmemory_%03u", i );

        hb_sharedmemory_create( sharedmemory_name, 10240, &process_handler->sharedmemory );

        Sleep( 100 ); //hack
    }

    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_grid_process_handler_t * process_handler = process_handlers + i;

        HANDLE hThread = process_handler->hThread;
        WaitForSingleObject( hThread, INFINITE );
    }

    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_grid_process_handler_t * process_handler = process_handlers + i;

        HANDLE hThread = process_handler->hThread;
        CloseHandle( hThread );

        hb_sharedmemory_destroy( &process_handler->sharedmemory );
    }

    HB_DELETE( process_handlers );

    WSACleanup();

    return EXIT_SUCCESS;
}