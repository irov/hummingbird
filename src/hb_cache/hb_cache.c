#include "hb_cache.h"

#include "hb_config/hb_config.h"
#include "hb_log/hb_log.h"
#include "hb_utils/hb_sha1.h"
#include "hb_utils/hb_base64.h"

#include "hiredis.h"

#include <string.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

//////////////////////////////////////////////////////////////////////////
redisContext * g_redis_context = HB_NULLPTR;
//////////////////////////////////////////////////////////////////////////
int hb_cache_initialze( const char * _uri, uint32_t _port )
{
    redisContext * c = redisConnect( _uri, _port ); //

    g_redis_context = c;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_cache_finalize()
{
    redisFree( g_redis_context );
    g_redis_context = HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
int hb_cache_set_binary_value( const char * _key, const void * _value, size_t _size )
{
    redisReply * reply = redisCommand( g_redis_context, "SET %s %b", _key, _value, _size );
    freeReplyObject( reply );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_cache_set_string_value( const char * _key, const char * _value, size_t _size )
{
    redisReply * reply = redisCommand( g_redis_context, "SET %s %b", _key, _value, _size );
    freeReplyObject( reply );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_cache_set_int64_value( const char * _key, int64_t _value )
{
    redisReply * reply = redisCommand( g_redis_context, "SET %s %" PRIX64, _key, _value );
    freeReplyObject( reply );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_cache_get_binary_value( const char * _key, void * _value, size_t _capacity, size_t * _size )
{
    redisReply * reply = redisCommand( g_redis_context, "GET %s", _key );

    if( reply->len > _capacity )
    {
        freeReplyObject( reply );

        return 0;
    }

    memcpy( _value, reply->str, reply->len );

    *_size = reply->len;

    freeReplyObject( reply );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_cache_get_string_value( const char * _key, char * _value, size_t _capacity, size_t * _size )
{
    redisReply * reply = redisCommand( g_redis_context, "GET %s", _key );

    if( reply->len > _capacity )
    {
        freeReplyObject( reply );

        return 0;
    }

    memcpy( _value, reply->str, reply->len );

    *_size = reply->len;

    freeReplyObject( reply );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_cache_get_int64_value( const char * _key, int64_t * _value )
{
    redisReply * reply = redisCommand( g_redis_context, "GET %s", _key );

    *_value = reply->integer;

    return 1;
}
//////////////////////////////////////////////////////////////////////////