#include "hb_leaderboard.h"

#include "hb_log/hb_log.h"
#include "hb_utils/hb_oid.h"

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_leaderboard_set( hb_cache_handle_t * _cache, const hb_oid_t _poid, const hb_oid_t _uoid, uint32_t _score )
{
    hb_oid16_t poid16;
    hb_oid_base16_encode( _poid, &poid16 );

    hb_oid16_t uoid16;
    hb_oid_base16_encode( _uoid, &uoid16 );

    if( hb_cache_zadd( _cache, poid16, sizeof( hb_oid16_t ), uoid16, sizeof( hb_oid16_t ), _score ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    HB_LOG_MESSAGE_INFO( "leaderboard", "set project '%.*s' user '%.*s' score '%u'"
        , sizeof( hb_oid16_t )
        , poid16
        , sizeof( hb_oid16_t )
        , uoid16
        , _score
    );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_leaderboard_get( hb_cache_handle_t * _cache, const hb_oid_t _poid, uint32_t _begin, uint32_t _end, hb_oid_t * _oids, uint32_t * _count )
{
    HB_UNUSED( _oids );
    HB_UNUSED( _count );

    hb_oid16_t poid16;
    hb_oid_base16_encode( _poid, &poid16 );

    if( hb_cache_zrevrange( _cache, poid16, sizeof( hb_oid16_t ), _begin, _end, HB_NULLPTR, HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
