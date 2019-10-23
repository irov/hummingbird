#ifndef HB_HTTPOPT_H_
#define HB_HTTPOPT_H_

#include "hb_config/hb_config.h"

hb_result_t hb_httpopt( const char * _http, size_t _size, const char * _name, const char ** _value, size_t * _valuesize );

#endif