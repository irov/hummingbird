#ifndef HB_SCRIPT_H_
#define HB_SCRIPT_H_

#include "hb_config/hb_config.h"

int hb_script_initialize( size_t _memorylimit, size_t _calllimit );
void hb_script_finalize();

int hb_script_user_initialize( const char * _user, const char * _db, const char * _collection );
void hb_script_user_finalize();

int hb_script_user_load( const void * _buffer, size_t _size );
int hb_script_user_call( const char * _method, size_t _methodsize, const char * _data, size_t _datasize, char * _result, size_t _capacity, size_t * _resultsize );

#endif
