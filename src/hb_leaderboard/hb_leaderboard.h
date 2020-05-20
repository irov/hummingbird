#ifndef HB_LEADERBOARD_H_
#define HB_LEADERBOARD_H_

#include "hb_config/hb_config.h"

#include "hb_cache/hb_cache.h"
#include "hb_db/hb_db.h"

hb_result_t hb_leaderboard_set( const hb_db_client_handle_t * _client, hb_cache_handle_t * _cache, const hb_oid_t * _poid, const hb_oid_t * _uoid, uint32_t _score );
hb_result_t hb_leaderboard_get_global( hb_cache_handle_t * _cache, const hb_oid_t * _poid, uint32_t _begin, uint32_t _end, hb_oid_t * _oids, uint32_t * _scores, uint32_t * _count );
hb_result_t hb_leaderboard_get_user( hb_cache_handle_t * _cache, const hb_oid_t * _poid, const hb_oid_t * _uoid, uint32_t * _score );

#endif