#ifndef HB_MULTIPART_H_
#define HB_MULTIPART_H_

#include "hb_config/hb_config.h"

typedef struct multipart_params_desc_t
{
    const char * key;
    size_t key_size;
    const void * memory_begin;
    const void * memory_end;
} multipart_params_desc_t;

int hb_multipart_parse( const void * _boundary, size_t _boundarysize, multipart_params_desc_t * _params, size_t _capacity, const void * _buffer, size_t _buffersize, uint32_t * _count );

#endif