#ifndef HB_LEADERBOARD_H_
#define HB_LEADERBOARD_H_

#include "hb_config/hb_config.h"

#include "hb_cache/hb_cache.h"

hb_result_t hb_leaderboard_set( hb_cache_handle_t * _cache, const hb_oid_t * _poid, const hb_oid_t * _uoid, uint32_t _score );
hb_result_t hb_leaderboard_get( hb_cache_handle_t * _cache, const hb_oid_t * _poid, uint32_t _begin, uint32_t _end, hb_oid_t * _oids, uint32_t * _scores, uint32_t * _count );

#endif