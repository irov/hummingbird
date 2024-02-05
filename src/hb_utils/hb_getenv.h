#ifndef HB_GETENV_H_
#define HB_GETENV_H_

#include "hb_config/hb_config.h"

hb_result_t hb_getenv( const char * _name, char * const _value, size_t _capacity );
hb_result_t hb_getenvi( const char * _name, int64_t * const _value );

#endif