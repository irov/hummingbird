#include "hb_cache.h"

#include "hb_memory/hb_memory.h"
#include "hb_token/hb_token.h"
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
#elif defined(HB_PLATFORM_LINUX)
#include <sys/time.h>
#elif defined(HB_PLATFORM_OSX)
#include <sys/time.h>
#endif

//////////////////////////////////////////////////////////////////////////
typedef struct hb_cache_handle_t
{
    redisContext * context;
} hb_cache_handle_t;
//////////////////////////////////////////////////////////////////////////
static const char * __hb_redis_reply_str( int32_t _type )
{
    const char * reply_str[15] = {"unknown", "string", "array", "integer", "nil", "status", "error", "double", "bool", "map", "set", "attr", "push", "bignum", "verb"};

    const char * str = reply_str[_type];

    return str;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_cache_create( const char * _uri, uint16_t _port, uint32_t _timeout, hb_cache_handle_t ** _handle )
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

    hb_cache_handle_t * handle = HB_NEW( hb_cache_handle_t );
    handle->context = c;

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_cache_destroy( hb_cache_handle_t * _handle )
{
    redisFree( _handle->context );

    HB_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_cache_set_value( const hb_cache_handle_t * _cache, const void * _key, hb_size_t _keysize, const void * _value, hb_size_t _valuesize )
{
    if( _keysize == HB_UNKNOWN_STRING_SIZE )
    {
        _keysize = strlen( (const char *)_key );
    }

    redisReply * reply = redisCommand( _cache->context, "SET %b %b", _key, _keysize, _value, _valuesize );

    if( reply == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'SET' with error: '%s'"
            , _cache->context->errstr
        );

        return HB_FAILURE;
    }

    if( reply->type == REDIS_REPLY_ERROR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'SET' with error: '%s'"
            , reply->str
        );

        return HB_FAILURE;
    }

    freeReplyObject( reply );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_cache_get_value( const hb_cache_handle_t * _cache, const void * _key, hb_size_t _keysize, void * _value, hb_size_t _capacity, hb_size_t * _size )
{
    if( _keysize == HB_UNKNOWN_STRING_SIZE )
    {
        _keysize = strlen( (const char *)_key );
    }

    redisReply * reply = redisCommand( _cache->context, "GET %b", _key, _keysize );

    if( reply == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'GET' with error: '%s'"
            , _cache->context->errstr
        );

        return HB_FAILURE;
    }

    if( reply->type == REDIS_REPLY_ERROR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'GET' with error: '%s'"
            , reply->str
        );

        return HB_FAILURE;
    }

    if( reply->type == REDIS_REPLY_NIL )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'GET' return nil" );

        freeReplyObject( reply );

        return HB_FAILURE;
    }

    if( reply->type != REDIS_REPLY_STRING )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'GET' return '%s' not 'string'" 
            , __hb_redis_reply_str( reply->type )
        );

        freeReplyObject( reply );

        return HB_FAILURE;
    }

    if( reply->len > _capacity )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'GET' return '%s' len %zu more capacity %zu"
            , reply->str
            , reply->len
            , _capacity
        );

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
hb_result_t hb_cache_incrby_value( const hb_cache_handle_t * _cache, const void * _key, hb_size_t _keysize, uint64_t _increment, uint64_t * _value )
{
    if( _keysize == HB_UNKNOWN_STRING_SIZE )
    {
        _keysize = strlen( (const char *)_key );
    }

    redisReply * reply = redisCommand( _cache->context, "INCRBY %b %" SCNu64, _key, _keysize, _increment );

    if( reply == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'INCRBY' with error: '%s'"
            , _cache->context->errstr
        );

        return HB_FAILURE;
    }

    if( reply->type == REDIS_REPLY_ERROR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'INCRBY' with error: '%s'"
            , reply->str
        );

        return HB_FAILURE;
    }

    if( reply->type != REDIS_REPLY_INTEGER )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'INCRBY' type '%d' not integer"
            , reply->type
        );

        freeReplyObject( reply );

        return HB_FAILURE;
    }

    *_value = (uint64_t)reply->integer;

    freeReplyObject( reply );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_cache_expire_value( const hb_cache_handle_t * _cache, const void * _key, hb_size_t _keysize, uint32_t _seconds )
{
    if( _keysize == HB_UNKNOWN_STRING_SIZE )
    {
        _keysize = strlen( (const char *)_key );
    }

    redisReply * reply = redisCommand( _cache->context, "EXPIRE %b %u", _key, _keysize, _seconds );

    if( reply == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'EXPIRE' with error: '%s'"
            , _cache->context->errstr
        );

        return HB_FAILURE;
    }

    if( reply->type == REDIS_REPLY_ERROR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'EXPIRE' with error: '%s'"
            , reply->str
        );

        freeReplyObject( reply );

        return HB_FAILURE;
    }

    freeReplyObject( reply );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_cache_get_token( const hb_cache_handle_t * _cache, const char * _token, uint32_t _seconds, void * _value, hb_size_t _capacity, hb_size_t * _size )
{
    hb_token_t token;
    if( hb_token_base16_decode_string( _token, &token ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_cache_expire_value( _cache, token.value, sizeof( hb_token_t ), _seconds ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_cache_get_value( _cache, token.value, sizeof( hb_token_t ), _value, _capacity, _size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_cache_zadd( const hb_cache_handle_t * _cache, const void * _key, hb_size_t _keysize, const void * _value, hb_size_t _valuesize, uint32_t _score )
{
    if( _keysize == HB_UNKNOWN_STRING_SIZE )
    {
        _keysize = strlen( (const char *)_key );
    }

    if( _valuesize == HB_UNKNOWN_STRING_SIZE )
    {
        _valuesize = strlen( (const char *)_value );
    }

    redisReply * reply = redisCommand( _cache->context, "ZADD %b %u %b", _key, _keysize, _score, _value, _valuesize );

    if( reply == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'ZADD' with error: '%s'"
            , _cache->context->errstr
        );

        return HB_FAILURE;
    }

    if( reply->type == REDIS_REPLY_ERROR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'ZADD' with error: '%s'"
            , reply->str
        );

        return HB_FAILURE;
    }

    freeReplyObject( reply );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_cache_zrevrange( const hb_cache_handle_t * _cache, const void * _key, hb_size_t _keysize, uint32_t _begin, uint32_t _end, hb_cache_value_t * _values, uint32_t * _count )
{
    redisReply * reply = redisCommand( _cache->context, "ZREVRANGE %b %u %u WITHSCORES", _key, _keysize, _begin, _end );

    if( reply == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'ZREVRANGE' with error: '%s'"
            , _cache->context->errstr
        );

        return HB_FAILURE;
    }

    if( reply->type == REDIS_REPLY_ERROR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'ZREVRANGE' with error: '%s'"
            , reply->str
        );

        return HB_FAILURE;
    }

    if( reply->type != REDIS_REPLY_ARRAY )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'ZREVRANGE' return '%s' not array"
            , __hb_redis_reply_str( reply->type )
        );

        return HB_FAILURE;
    }

    for( hb_size_t index = 0; index != reply->elements; ++index )
    {
        struct redisReply * element = reply->element[index];

        hb_cache_value_t * value = _values + index;

        switch( element->type )
        {
        case REDIS_REPLY_INTEGER:
            {
                value->type = e_hb_cache_integer;

                value->integer = (int64_t)element->integer;
            }break;
        case REDIS_REPLY_DOUBLE:
            {
                value->type = e_hb_cache_double;

                value->real = element->dval;
            }break;
        case REDIS_REPLY_STRING:
            {
                value->type = e_hb_cache_string;

                if( element->len > 127 )
                {
                    return HB_FAILURE;
                }

                memcpy( value->string, element->str, element->len );
                value->string[element->len] = '\0';
            }break;
        default:
            {
                HB_LOG_MESSAGE_ERROR( "cache", "redis command 'ZREVRANGE' element %zu type '%s' not array"
                    , index
                    , __hb_redis_reply_str( reply->type )
                );

                return HB_FAILURE;
            }break;
        }
    }

    *_count = (uint32_t)reply->elements;

    freeReplyObject( reply );    

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_cache_zrevrank( const hb_cache_handle_t * _cache, const void * _key, hb_size_t _keysize, const void * _value, hb_size_t _valuesize, uint32_t * _score, hb_bool_t * _exist )
{
    redisReply * reply = redisCommand( _cache->context, "ZREVRANK %b %b", _key, _keysize, _value, _valuesize );

    if( reply == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'ZREVRANGE' with error: '%s'"
            , _cache->context->errstr
        );

        return HB_FAILURE;
    }

    if( reply->type == REDIS_REPLY_ERROR )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'ZREVRANK' with error: '%s'"
            , reply->str
        );

        return HB_FAILURE;
    }

    if( reply->type == REDIS_REPLY_NIL )
    {
        freeReplyObject( reply );

        *_exist = HB_FALSE;

        return HB_SUCCESSFUL;
    }

    if( reply->type != REDIS_REPLY_INTEGER )
    {
        HB_LOG_MESSAGE_ERROR( "cache", "redis command 'ZREVRANK' return type '%s' not 'integer'"
            , __hb_redis_reply_str( reply->type )
        );

        freeReplyObject( reply );

        return HB_FAILURE;
    }

    uint32_t score = (uint32_t)reply->integer;

    *_score = score;
    *_exist = HB_TRUE;

    freeReplyObject( reply );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////