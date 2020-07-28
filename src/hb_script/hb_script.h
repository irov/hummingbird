#ifndef HB_SCRIPT_H_
#define HB_SCRIPT_H_

#include "hb_db/hb_db.h"
#include "hb_matching/hb_matching.h"
#include "hb_cache/hb_cache.h"
#include "hb_json/hb_json.h"

#include "hb_config/hb_config.h"

typedef struct hb_script_handle_t hb_script_handle_t;

hb_result_t hb_script_initialize( const hb_cache_handle_t * _cache, const hb_db_client_handle_t * _client, size_t _memorylimit, size_t _calllimit, hb_uid_t _puid, hb_uid_t _uuid, hb_matching_handle_t * _matching, hb_script_handle_t ** _handle );
void hb_script_finalize( hb_script_handle_t * _handle );

hb_result_t hb_script_api_call( hb_script_handle_t * _handle, const char * _api, const char * _method, const hb_json_handle_t * _json, char * _result, size_t _capacity, size_t * _resultsize, hb_error_code_t * _code );
hb_result_t hb_script_api_call_data( hb_script_handle_t * _handle, const char * _api, const char * _method, const void * _data, size_t _datasize, char * _result, size_t _capacity, size_t * _resultsize, hb_error_code_t * _code );

typedef struct hb_script_stat_t
{
    size_t memory_used;
    uint32_t call_used;

}hb_script_stat_t;

void hb_script_stat( hb_script_handle_t * _handle, hb_script_stat_t * _stat );

#endif
