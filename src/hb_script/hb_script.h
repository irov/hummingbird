#ifndef HB_SCRIPT_H_
#define HB_SCRIPT_H_

#include "hb_config/hb_config.h"

#include "hb_db/hb_db.h"

hb_result_t hb_script_initialize( size_t _memorylimit, size_t _calllimit, const hb_db_collection_handle_t * _ucollection, const hb_db_collection_handle_t * _pcollection, hb_oid_t _uuid, hb_oid_t _puid );
void hb_script_finalize();

hb_result_t hb_script_load( const void * _buffer, size_t _size );
hb_result_t hb_script_server_call( const char * _method, const void * _data, size_t _datasize, char * _result, size_t _capacity, size_t * _resultsize, hb_bool_t * _successful );
hb_result_t hb_script_event_call( const char * _method, const void * _data, size_t _datasize );

#endif
