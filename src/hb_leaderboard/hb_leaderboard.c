#include "hb_leaderboard.h"

#include "hb_log/hb_log.h"
#include "hb_utils/hb_oid.h"
#include "hb_utils/hb_base16.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_leaderboard_set( hb_cache_handle_t * _cache, const hb_oid_t * _poid, const hb_oid_t * _uoid, uint32_t _score )
{
    hb_oid16_t poid16;
    hb_oid_base16_encode( _poid, &poid16 );

    hb_oid16_t uoid16;
    hb_oid_base16_encode( _uoid, &uoid16 );

    if( hb_cache_zadd( _cache, poid16.value, sizeof( hb_oid16_t ), uoid16.value, sizeof( hb_oid16_t ), _score ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    HB_LOG_MESSAGE_INFO( "leaderboard", "set project '%.*s' user '%.*s' score '%u'"
        , sizeof( hb_oid16_t )
        , poid16.value
        , sizeof( hb_oid16_t )
        , uoid16.value
        , _score
    );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_leaderboard_get( hb_cache_handle_t * _cache, const hb_oid_t * _poid, uint32_t _begin, uint32_t _end, hb_oid_t * _oids, uint32_t * _scores, uint32_t * _count )
{
    if( _begin > _end )
    {
        return HB_FAILURE;
    }

    if( _end - _begin > 16 )
    {
        return HB_FAILURE;
    }

    hb_oid16_t poid16;
    hb_oid_base16_encode( _poid, &poid16 );

    uint32_t values_count;
    hb_cache_value_t values[32];
    if( hb_cache_zrevrange( _cache, poid16.value, sizeof( hb_oid16_t ), _begin, _end, values, &values_count ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( values_count % 2 == 1 )
    {
        return HB_FAILURE;
    }

    for( uint32_t index = 0; index != values_count; index += 2 )
    {
        hb_cache_value_t * value0 = values + index + 0;
        hb_cache_value_t * value1 = values + index + 1;

        hb_oid_t * oid = _oids + index / 2;
        uint32_t * score = _scores + index / 2;

        if( hb_base16_decode( value0->string, HB_UNKNOWN_STRING_SIZE, oid, sizeof( hb_oid_t ), HB_NULLPTR ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        if( sscanf( value1->string, "%u", score ) != 1 )
        {
            return HB_FAILURE;
        }
    }

    *_count = values_count / 2;

    return HB_SUCCESSFUL;
}
