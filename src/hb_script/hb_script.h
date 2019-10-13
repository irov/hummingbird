#ifndef HB_SCRIPT_H_
#define HB_SCRIPT_H_

#include "hb_config/hb_config.h"

#include "hb_db/hb_db.h"

int hb_script_initialize( size_t _memorylimit, size_t _calllimit, const hb_db_collection_handle_t * _ucollection, const hb_db_collection_handle_t * _pcollection, const uint8_t * _uuid, const uint8_t * _puid );
void hb_script_finalize();

int hb_script_load( const void * _buffer, size_t _size );
int hb_script_call( const char * _method, const char * _data, size_t _datasize, char * _result, size_t _capacity, size_t * _resultsize );

#endif
