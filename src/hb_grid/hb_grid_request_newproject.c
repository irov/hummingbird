#include "hb_grid.h"

#include "hb_grid_process_newproject.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

int hb_grid_request_newproject( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const char * _token )
{
    HB_UNUSED( _request );

    hb_grid_process_newproject_in_data_t in_data;
    hb_token_base16_decode( _token, &in_data.token );

    hb_grid_process_newproject_out_data_t out_data;
    if( hb_grid_process_newproject( _process, &in_data, &out_data ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_pid16_t pid16;
    hb_base16_encode( &out_data.pid, sizeof( out_data.pid ), pid16, sizeof( pid16 ), HB_NULLPTR );

    size_t response_data_size = sprintf( _response, "{\"code\": 0, \"pid\": \"%.*s\"}"
        , (int)sizeof( pid16 )
        , pid16
    );

    *_size = response_data_size;

    return HTTP_OK;
}