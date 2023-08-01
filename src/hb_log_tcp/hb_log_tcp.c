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
static void __hb_log_tcp_observer( const char * _category, hb_log_level_t _level, const char * _file, uint32_t _line, const char * _message, void * _ud )
{
    HB_UNUSED( _file );
    HB_UNUSED( _line );

    hb_log_tcp_handle_t * handle = (hb_log_tcp_handle_t *)_ud;

    hb_time_t t;
    hb_time( &t );

    char message[2048];

#ifdef HB_DEBUG
    int32_t message_size = sprintf( message, "{\"time\":%" SCNu64 ", \"category\":\"%s\", \"level\":%u, \"file\":\"%s\", \"line\":%u, \"message\":\"%s\"}\r\n", t, _category, _level, _file, _line, _message );
#else
    int32_t message_size = sprintf( message, "{\"time\":%" SCNu64 ", \"category\":\"%s\", \"level\":%u, \"message\":\"%s\"}\r\n", t, _category, _level, _message );
#endif

    bufferevent_write( handle->bev_cnn, message, message_size );

    event_base_dispatch( handle->base );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_log_tcp_initialize( const char * _url, uint16_t _port )
{
#ifdef HB_PLATFORM_WINDOWS
    const WORD wVersionRequested = MAKEWORD( 2, 2 );

    WSADATA wsaData;
    int32_t err = WSAStartup( wVersionRequested, &wsaData );

    if( err != 0 )
    {
        return EXIT_FAILURE;
    }
#endif

    struct hostent * h = gethostbyname( _url );

    if( h == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    if( h->h_addrtype != AF_INET )
    {
        return HB_FAILURE;
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons( _port ); //5044
    sin.sin_addr = *(struct in_addr *)h->h_addr;

    struct event_base * base = event_base_new();
    struct bufferevent * bev_cnn = bufferevent_socket_new( base, -1, BEV_OPT_CLOSE_ON_FREE );

    if( bev_cnn == HB_NULLPTR )
    {
        event_base_free( base );

        return HB_FAILURE;
    }

    if( bufferevent_socket_connect( bev_cnn, (const struct sockaddr *)&sin, sizeof( sin ) ) == -1 )
    {
        bufferevent_free( bev_cnn );
        event_base_free( base );

        return HB_FAILURE;
    }

    bufferevent_enable( bev_cnn, EV_WRITE );
    bufferevent_disable( bev_cnn, EV_READ );

    hb_log_tcp_handle_t * handle = HB_NEW( hb_log_tcp_handle_t );
    handle->base = base;
    handle->bev_cnn = bev_cnn;

    if( hb_log_add_observer( HB_NULLPTR, HB_LOG_ERROR, &__hb_log_tcp_observer, handle ) == HB_FAILURE )
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
    hb_log_tcp_handle_t * handle;
    if( hb_log_remove_observer( &__hb_log_tcp_observer, &handle ) == HB_SUCCESSFUL )
    {
        bufferevent_free( handle->bev_cnn );
        event_base_free( handle->base );

        HB_DELETE( handle );
    }

#ifdef HB_PLATFORM_WINDOWS
    WSACleanup();
#endif
}
//////////////////////////////////////////////////////////////////////////