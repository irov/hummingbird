#ifndef HB_SCRIPT_H_
#define HB_SCRIPT_H_

#include "hb_config/hb_config.h"

#include "hb_db/hb_db.h"
#include "hb_matching/hb_matching.h"

typedef struct hb_script_handle_t hb_script_handle_t;

hb_result_t hb_script_initialize( size_t _memorylimit, size_t _calllimit, const hb_oid_t _puid, const hb_oid_t _uuid, hb_matching_t * _matching, hb_script_handle_t ** _handle );
void hb_script_finalize( hb_script_handle_t * _handle );

typedef struct hb_script_stat_t
{
    size_t memory_used;
    uint32_t call_used;

}hb_script_stat_t;

void hb_script_stat( hb_script_handle_t * _handle, hb_script_stat_t * _stat );

hb_result_t hb_script_load( hb_script_handle_t * _handle, const void * _buffer, size_t _size );
hb_result_t hb_script_api_call( hb_script_handle_t * _handle, const char * _method, const void * _data, size_t _datasize, char * _result, size_t _capacity, size_t * _resultsize, hb_error_code_t * _code );
hb_result_t hb_script_event_call( hb_script_handle_t * _handle, const char * _method, const void * _data, size_t _datasize );
hb_result_t hb_script_command_call( hb_script_handle_t * _handle, const char * _method, const void * _data, size_t _datasize, char * _result, size_t _capacity, size_t * _resultsize, hb_error_code_t * _code );

#endif
