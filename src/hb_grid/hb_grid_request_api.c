#include "hb_grid.h"
#include "hb_grid_process_script_api.h"

#include "hb_http/hb_http.h"
#include "hb_token/hb_token.h"
#include "hb_utils/hb_base64.h"

#include <string.h>

int hb_grid_request_api( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const char * _token, const char * _method )
{
    HB_UNUSED( _process );

    hb_grid_process_script_api_in_data_t in_data;
    if( hb_token_base16_decode_string( _token, &in_data.token ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    strcpy( in_data.method, _method );

    if( hb_http_get_request_data( _request, in_data.data, HB_DATA_MAX_SIZE, &in_data.data_size ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_script_api_out_data_t out_data;
    if( hb_grid_process_script_api( _process, &in_data, &out_data ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( out_data.successful == HB_TRUE )
    {
        size_t response_data_size = sprintf( _response, "{\"code\": 0, \"successful\": true, \"data\": %.*s, \"stat\": {\"memory_used\":%zu, \"call_used\":%u}}"
            , (int)out_data.response_size
            , out_data.response_data
            , out_data.memory_used
            , out_data.call_used
        );

        *_size = response_data_size;
    }
    else
    {
        size_t response_data_size = sprintf( _response, "{\"code\": 0, \"successful\": false, \"method_found\": %s}"
            , out_data.method_found == HB_TRUE ? "true" : "false"
        );

        *_size = response_data_size;
    }

    return HTTP_OK;
}