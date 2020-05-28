#include "hb_leaderboard.h"

#include "hb_log/hb_log.h"
#include "hb_utils/hb_base16.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_leaderboard_set( const hb_db_client_handle_t * _client, hb_cache_handle_t * _cache, hb_uid_t _poid, hb_uid_t _uoid, uint32_t _score )
{
    hb_db_values_handle_t * values_update;
    if( hb_db_create_values( &values_update ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_int32_value( values_update, "leaderboard_score", HB_UNKNOWN_STRING_SIZE, (int32_t)_score );

    if( hb_db_update_values_by_name( _client, "hb_users", _uoid, values_update ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( values_update );

    if( hb_cache_zadd( _cache, &_poid, sizeof( hb_uid_t ), &_uoid, sizeof( hb_uid_t ), _score ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    HB_LOG_MESSAGE_INFO( "leaderboard", "set project '%d' user '%d' score '%u'"
        , _poid
        , _uoid
        , _score
    );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_leaderboard_get_global( hb_cache_handle_t * _cache, hb_uid_t _poid, uint32_t _begin, uint32_t _end, hb_uid_t * _oids, uint32_t * _scores, uint32_t * _count )
{
    if( _begin > _end )
    {
        return HB_FAILURE;
    }

    if( _end - _begin > 16 )
    {
        return HB_FAILURE;
    }

    uint32_t values_count;
    hb_cache_value_t values[32];
    if( hb_cache_zrevrange( _cache, &_poid, sizeof( hb_uid_t ), _begin, _end, values, &values_count ) == HB_FAILURE )
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

        hb_uid_t * oid = _oids + index / 2;
        uint32_t * score = _scores + index / 2;

        if( hb_base16_decode( value0->string, HB_UNKNOWN_STRING_SIZE, oid, sizeof( hb_uid_t ), HB_NULLPTR ) == HB_FAILURE )
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
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_leaderboard_get_rank( hb_cache_handle_t * _cache, hb_uid_t _poid, hb_uid_t _uoid, uint32_t * _rank, hb_bool_t * _exist )
{
    uint32_t rank;
    hb_bool_t exist;
    if( hb_cache_zrevrank( _cache, &_poid, sizeof( hb_uid_t ), &_uoid, sizeof( hb_uid_t ), &rank, &exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    *_rank = rank;
    *_exist = exist;

    return HB_SUCCESSFUL;
}
