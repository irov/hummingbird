#ifndef HB_FILE_H_
#define HB_FILE_H_

#include "hb_config/hb_config.h"

hb_result_t hb_file_read( const char * _path, void * _buffer, hb_size_t _capacity, hb_size_t * _size );
hb_result_t hb_file_read_text( const char * _path, char * _buffer, hb_size_t _capacity, hb_size_t * _size );

#endif