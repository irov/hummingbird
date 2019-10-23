#ifndef HB_MULTIPART_H_
#define HB_MULTIPART_H_

#include "hb_config/hb_config.h"

typedef struct multipart_params_handle_t
{
    const char * key;
    size_t key_size;

    const char * data;
    size_t data_size;
} multipart_params_handle_t;

hb_result_t hb_multipart_parse( const void * _boundary, size_t _boundarysize, const void * _buffer, size_t _buffersize, multipart_params_handle_t * _params, size_t _capacity, uint32_t * _count );
hb_result_t hb_multipart_get_value( multipart_params_handle_t * _handles, uint32_t _count, const char * _key, const void ** _data, size_t * _size );

#endif