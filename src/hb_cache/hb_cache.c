#include "hb_cache.h"

#include "hb_config/hb_config.h"
#include "hb_log/hb_log.h"

#include "hiredis.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
redisContext * g_redis_context = HB_NULLPTR;
//////////////////////////////////////////////////////////////////////////
int hb_cache_initialze( const char * _uri, uint32_t _port )
{
    redisContext * c = redisConnect( _uri, _port ); //6379

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
int hb_cache_set_value( const void * _key, size_t _keysize, const void * _value, size_t _size )
{
    redisReply * reply = redisCommand( g_redis_context, "SET %b %b", _key, _keysize, _value, _size );
    freeReplyObject( reply );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_cache_expire_value( const void * _key, size_t _keysize, uint32_t _seconds )
{
    redisReply * reply = redisCommand( g_redis_context, "EXPIRE %b %u", _key, _keysize, _seconds );
    freeReplyObject( reply );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_cache_get_value( const void * _key, size_t _keysize, void * _value, size_t _capacity, size_t * _size )
{
    redisReply * reply = redisCommand( g_redis_context, "GET %b", _key, _keysize );

    if( reply->type == REDIS_REPLY_NIL )
    {
        return 0;
    }

    if( reply->len > _capacity )
    {
        freeReplyObject( reply );

        return 0;
    }

    memcpy( _value, reply->str, reply->len );

    if( _size != HB_NULLPTR )
    {
        *_size = reply->len;
    }

    freeReplyObject( reply );

    return 1;
}
//////////////////////////////////////////////////////////////////////////