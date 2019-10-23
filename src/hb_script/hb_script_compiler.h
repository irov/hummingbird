#ifndef HB_SCRIPT_COMPILER_H_
#define HB_SCRIPT_COMPILER_H_

#include "hb_config/hb_config.h"

hb_result_t hb_script_compiler( const void * _buffer, size_t _size, void * _compile, size_t _capacity, size_t * _compilesize );

#endif
