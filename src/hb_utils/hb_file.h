#ifndef HB_FILE_H_
#define HB_FILE_H_

#include "hb_config/hb_config.h"

hb_result_t hb_file_read( const char * _path, void * _buffer, size_t _capacity, size_t * _size );

#endif