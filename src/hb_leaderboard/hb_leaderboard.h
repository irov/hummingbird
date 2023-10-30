#ifndef HB_LEADERBOARD_H_
#define HB_LEADERBOARD_H_

#include "hb_config/hb_config.h"

#include "hb_cache/hb_cache.h"
#include "hb_db/hb_db.h"

hb_result_t hb_leaderboard_set( const hb_db_client_handle_t * _client, hb_cache_handle_t * _cache, hb_uid_t _puid, hb_uid_t _uuid, uint64_t _score );
hb_result_t hb_leaderboard_get_global( hb_cache_handle_t * _cache, hb_uid_t _puid, uint32_t _begin, uint32_t _end, hb_uid_t * _uids, uint64_t * _scores, uint32_t * _count );
hb_result_t hb_leaderboard_get_rank( hb_cache_handle_t * _cache, hb_uid_t _puid, hb_uid_t _uoid, uint32_t * _rank, hb_bool_t * _exist );

#endif