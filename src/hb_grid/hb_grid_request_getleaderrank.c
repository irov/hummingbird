#include "hb_grid.h"

#include "hb_grid_process_getleaderrank.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

int hb_grid_request_getleaderrank( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const char * _token )
{
    HB_UNUSED( _request );

    hb_bool_t required_successful = HB_TRUE;

    hb_grid_process_getleaderrank_in_data_t in_data;

    if( hb_token_base16_decode_string( _token, &in_data.token ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( required_successful == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_getleaderrank_out_data_t out_data;
    if( hb_grid_process_getleaderrank( _process, &in_data, &out_data ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( out_data.exist == HB_TRUE )
    {
        *_size = sprintf( _response, "{\"code\":0,\"rank\":%u}"
            , out_data.rank 
        );
    }
    else
    {
        *_size = sprintf( _response, "{\"code\":1}" );
    }

    return HTTP_OK;
}