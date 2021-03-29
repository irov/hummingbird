#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include "hb_log_tcp.h"

#include "hb_memory/hb_memory.h"
#include "hb_utils/hb_time.h"

#include "evhttp.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_log_tcp_handle_t
{
    struct event_base * base;
    struct bufferevent * bev_cnn;
}hb_log_tcp_handle_t;
//////////////////////////////////////////////////////////////////////////
static hb_log_tcp_handle_t * g_log_tcp_handle = HB_NULLPTR;
//////////////////////////////////////////////////////////////////////////
static void __hb_log_tcp_observer( const char * _category, hb_log_level_t _level, const char * _file, uint32_t _line, const char * _message, void * _ud )
{
    HB_UNUSED( _ud );
    HB_UNUSED( _file );
    HB_UNUSED( _line );

    hb_time_t t;
    hb_time( &t );

    char message[2048];

#ifdef HB_DEBUG
    int32_t message_size = sprintf( message, "{\"time\":%" SCNu64 ", \"category\":\"%s\", \"level\":%u, \"file\":\"%s\", \"line\":%u, \"message\":\"%s\"}\r\n", t, _category, _level, _file, _line, _message );
#else
    int32_t message_size = sprintf( message, "{\"time\":%" SCNu64 ", \"category\":\"%s\", \"level\":%u, \"message\":\"%s\"}\r\n", t, _category, _level, _message );
#endif

    bufferevent_write( g_log_tcp_handle->bev_cnn, message, message_size );

    event_base_dispatch( g_log_tcp_handle->base );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_log_tcp_initialize( const char * _url, uint16_t _port )
{
#ifdef WIN32
    const WORD wVersionRequested = MAKEWORD( 2, 2 );

    WSADATA wsaData;
    int32_t err = WSAStartup( wVersionRequested, &wsaData );

    if( err != 0 )
    {
        return EXIT_FAILURE;
    }
#endif

    struct hostent * h = gethostbyname( _url );

    if( !h )
    {
        return HB_FAILURE;
    }

    if( h->h_addrtype != AF_INET )
    {
        fprintf( stderr, "No ipv4 support, sorry." );
        return HB_FAILURE;
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons( _port ); //5044
    sin.sin_addr = *(struct in_addr *)h->h_addr;

    struct event_base * base = event_base_new();
    struct bufferevent * bev_cnn = bufferevent_socket_new( base, -1, BEV_OPT_CLOSE_ON_FREE );

    int32_t error_connect = bufferevent_socket_connect( bev_cnn, (const struct sockaddr *) & sin, sizeof( sin ) );

    if( error_connect != 0 )
    {
        bufferevent_free( bev_cnn );
        event_base_free( base );

        return HB_FAILURE;
    }

    bufferevent_enable( bev_cnn, EV_WRITE );
    bufferevent_disable( bev_cnn, EV_READ );

    g_log_tcp_handle = HB_NEW( hb_log_tcp_handle_t );
    g_log_tcp_handle->base = base;
    g_log_tcp_handle->bev_cnn = bev_cnn;

    if( hb_log_add_observer( HB_NULLPTR, HB_LOG_ERROR, &__hb_log_tcp_observer, HB_NULLPTR ) == HB_FAILURE )
    {
        bufferevent_free( bev_cnn );
        event_base_free( base );

        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_log_tcp_finalize()
{
    if( g_log_tcp_handle != HB_NULLPTR )
    {
        hb_log_remove_observer( &__hb_log_tcp_observer, HB_NULLPTR );

        bufferevent_free( g_log_tcp_handle->bev_cnn );
        event_base_free( g_log_tcp_handle->base );

        HB_DELETE( g_log_tcp_handle );
        g_log_tcp_handle = HB_NULLPTR;
    }

#ifdef WIN32
    WSACleanup();
#endif
}