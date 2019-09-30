#ifndef HB_SCRIPT_H_
#define HB_SCRIPT_H_

#include "hb_config/hb_config.h"

int hb_script_initialize();
void hb_script_finalize();

int hb_script_load( const void * _buffer, size_t _size );
int hb_script_call( const char * _method, const char * _data, char * _result, size_t _capacity );

#endif
