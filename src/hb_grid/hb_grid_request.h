#ifndef HB_GRID_REQUEST_H_
#define HB_GRID_REQUEST_H_

#include "hb_grid.h"

#include "hb_utils/hb_multipart.h"

hb_result_t hb_grid_get_request_params( struct evhttp_request * _request, multipart_params_handle_t * _params, uint32_t _capacity, uint32_t * _count );
hb_result_t hb_grid_get_request_data( struct evhttp_request * _request, void * _buffer, size_t _capacity, size_t * _size );
hb_result_t hb_grid_get_request_header( struct evhttp_request * _request, const char * _header, const char ** _value );

#endif