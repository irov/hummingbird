#include "hb_cache.h"

#include "hb_log/hb_log.h"

#pragma warning( push )
#pragma warning( disable : 4244 )
#include "hiredis.h"
#pragma warning( pop )

#include <string.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#if defined(HB_PLATFORM_WINDOWS)
#include <WinSock2.h>
#else defined(HB_PLATFORM_LINUX)
#include <sys/time.h>
#endif

//////////////////////////////////////////////////////////////////////////
redisContext * g_redis_context = HB_NULLPTR;
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_cache_available()
{
    if( g_redis_context == HB_NULLPTR )
    {
        return HB_FALSE;
    }

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_cache_initialize( const char * _uri, uint16_t _port, uint32_t _timeout )
{
    struct timeval tv_timeout;
    tv_timeout.tv_sec = _timeout / 1000;
    tv_timeout.tv_usec = (_timeout % 1000) * 1000;

    redisContext * c = redisConnectWithTimeout( _uri, _port, tv_timeout ); //6379

    if( c == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    if( c->err != REDIS_OK )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "connect url:'%s' port: %u get error [%s:%d]"
            , _uri
            , _port
            , c->errstr
            , c->err
        );

        return HB_FAILURE;
    }

    if( redisEnableKeepAlive( c ) != REDIS_OK )
    {
        redisFree( c );

        return HB_FAILURE;
    }

    g_redis_context = c;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_cache_finalize()
{
    if( g_redis_context != HB_NULLPTR )
    {
        redisFree( g_redis_context );
        g_redis_context = HB_NULLPTR;
    }
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_cache_set_value( const void * _key, size_t _keysize, const void * _value, size_t _size )
{
    if( g_redis_context == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    if( _keysize == HB_UNKNOWN_STRING_SIZE )
    {
        _keysize = strlen( (const char *)_key );
    }

    redisReply * reply = redisCommand( g_redis_context, "SET %b %b", _key, _keysize, _value, _size );

    if( reply == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command set with error: '%s'"
            , g_redis_context->errstr
        );

        return HB_FAILURE;
    }

    freeReplyObject( reply );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_cache_get_value( const void * _key, size_t _keysize, void * _value, size_t _capacity, size_t * _size )
{
    if( g_redis_context == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    if( _keysize == HB_UNKNOWN_STRING_SIZE )
    {
        _keysize = strlen( (const char *)_key );
    }

    redisReply * reply = redisCommand( g_redis_context, "GET %b", _key, _keysize );

    if( reply == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command get with error: '%s'"
            , g_redis_context->errstr
        );

        return HB_FAILURE;
    }

    if( reply->type == REDIS_REPLY_NIL )
    {
        freeReplyObject( reply );

        return HB_FAILURE;
    }

    if( reply->len > _capacity )
    {
        freeReplyObject( reply );

        return HB_FAILURE;
    }

    memcpy( _value, reply->str, reply->len );

    if( _size != HB_NULLPTR )
    {
        *_size = reply->len;
    }

    freeReplyObject( reply );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_cache_incrby_value( const void * _key, size_t _keysize, uint64_t _increment, uint64_t * _value )
{
    if( g_redis_context == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    if( _keysize == HB_UNKNOWN_STRING_SIZE )
    {
        _keysize = strlen( (const char *)_key );
    }

    redisReply * reply = redisCommand( g_redis_context, "INCRBY %b %" SCNu64, _key, _keysize, _increment );

    if( reply == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command incrby with error: '%s'"
            , g_redis_context->errstr
        );

        return HB_FAILURE;
    }

    if( reply->type != REDIS_REPLY_INTEGER )
    {
        freeReplyObject( reply );

        return HB_FAILURE;
    }

    *_value = (uint64_t)reply->integer;

    freeReplyObject( reply );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_cache_expire_value( const void * _key, size_t _keysize, uint32_t _seconds )
{
    if( g_redis_context == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    if( _keysize == HB_UNKNOWN_STRING_SIZE )
    {
        _keysize = strlen( (const char *)_key );
    }

    redisReply * reply = redisCommand( g_redis_context, "EXPIRE %b %u", _key, _keysize, _seconds );

    if( reply == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command expire with error: '%s'"
            , g_redis_context->errstr
        );

        return HB_FAILURE;
    }

    freeReplyObject( reply );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////