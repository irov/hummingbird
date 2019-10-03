#include "hb_grid.h"

#include "evhttp.h"
#include "event2/thread.h"

#include <process.h>

//////////////////////////////////////////////////////////////////////////
static void __hb_ev_on_request( struct evhttp_request * _request, void * _ud )
{
    HB_UNUSED( _request );
    HB_UNUSED( _ud );

    enum evhttp_cmd_type command_type = evhttp_request_get_command( _request );
    HB_UNUSED( command_type );

    struct evkeyvalq * keyval = evhttp_request_get_input_headers( _request );
    HB_UNUSED( keyval );

    struct evbuffer * input_buffer = evhttp_request_get_input_buffer( _request );
    HB_UNUSED( input_buffer );

    uint8_t copyout_buffer[2048];
    ev_ssize_t copyout_buffer_size = evbuffer_copyout( input_buffer, copyout_buffer, 2048 );
    HB_UNUSED( copyout_buffer_size );

    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );
    
    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    evbuffer_add_printf( output_buffer, "<html><body><center><h1>%.*s</h1></center></body></html>", copyout_buffer_size, copyout_buffer );

    evhttp_send_reply( _request, HTTP_OK, "", output_buffer );
}
//////////////////////////////////////////////////////////////////////////
static uint32_t __stdcall __hb_ev_thread_base( void * _ud )
{
    HB_UNUSED( _ud );

    evutil_socket_t * ev_socket = (evutil_socket_t *)_ud;

    char const server_address[] = "127.0.0.1";
    //char const server_address[] = "0.0.0.0";
    ev_uint16_t server_port = 5555;

    struct event_base * base = event_base_new();        
    HB_UNUSED( base );

    struct evhttp * http_server = evhttp_new( base );
    HB_UNUSED( http_server );

    evhttp_set_gencb( http_server, &__hb_ev_on_request, HB_NULLPTR );

    if( *ev_socket == -1 )
    {
        struct evhttp_bound_socket * bound_socket = evhttp_bind_socket_with_handle( http_server, server_address, server_port );
        HB_UNUSED( bound_socket );

        *ev_socket = evhttp_bound_socket_get_fd( bound_socket );
    }
    else
    {
        evhttp_accept_socket( http_server, *ev_socket );
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

    //char const server_address[] = "127.0.0.1";
    //char const server_address[] = "0.0.0.0";
    //ev_uint16_t server_port = 5555;

    WORD wVersionRequested = MAKEWORD( 2, 2 );

    WSADATA wsaData;
    int err = WSAStartup( wVersionRequested, &wsaData );

    if( err != 0 )
    {
        return EXIT_FAILURE;
    }

    HANDLE hThreads[8];

    evutil_socket_t ev_socket = -1;
    for( uint32_t i = 0; i != 8; ++i )
    {
        Sleep( 500 ); //hack

        uint32_t threadId;
        HANDLE hThread = (HANDLE)_beginthreadex( HB_NULLPTR, 0, &__hb_ev_thread_base, &ev_socket, 0, &threadId );

        hThreads[i] = hThread;
    }

    for( uint32_t i = 0; i != 8; ++i )
    {
        HANDLE hThread = hThreads[i];

        WaitForSingleObject( hThread, INFINITE );
        CloseHandle( hThread );
    }

    WSACleanup();

    return EXIT_SUCCESS;
}