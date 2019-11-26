#ifndef HB_SCRIPT_H_
#define HB_SCRIPT_H_

#include "hb_config/hb_config.h"

#include "hb_db/hb_db.h"

hb_result_t hb_script_initialize( size_t _memorylimit, size_t _calllimit, const hb_oid_t _uuid, const hb_oid_t _puid );
void hb_script_finalize();

typedef struct hb_script_stat_t
{
    size_t memory_used;
    uint32_t call_used;

}hb_script_stat_t;

void hb_script_stat( hb_script_stat_t * _stat );

hb_result_t hb_script_load( const void * _buffer, size_t _size );
hb_result_t hb_script_server_call( const char * _method, const void * _data, size_t _datasize, char * _result, size_t _capacity, size_t * _resultsize, hb_bool_t * _successful, hb_error_code_e * _code );
hb_result_t hb_script_event_call( const char * _method, const void * _data, size_t _datasize );

#endif
