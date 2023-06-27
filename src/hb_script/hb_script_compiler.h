#ifndef HB_SCRIPT_COMPILER_H_
#define HB_SCRIPT_COMPILER_H_

#include "hb_config/hb_config.h"

hb_result_t hb_script_compiler( const char * _source, hb_size_t _size, void * _compile, hb_size_t _capacity, hb_size_t * const _compilesize );

#endif
