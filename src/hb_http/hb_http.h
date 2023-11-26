#ifndef HB_HTTP_H_
#define HB_HTTP_H_

#include "hb_config/hb_config.h"
#include "hb_utils/hb_multipart.h"
#include "hb_json/hb_json.h"

#include "evhttp.h"

hb_result_t hb_http_get_request_params( struct evhttp_request * _request, multipart_params_handle_t * _params, uint32_t _capacity, uint32_t * _count );
hb_result_t hb_http_get_request_data( struct evhttp_request * _request, void * _buffer, hb_size_t _capacity, hb_size_t * _size );
hb_bool_t hb_http_is_request_json( struct evhttp_request * _request );
hb_result_t hb_http_get_request_header( struct evhttp_request * _request, const char * _header, const char ** _value );

#endif
