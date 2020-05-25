#include "hb_grid.h"

#include "hb_grid_process_newmessageschannel.h"
#include "hb_grid_process_script_event.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_getmessageschannel( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _args )
{
    HB_UNUSED( _request );

    const char * account_token = _args->arg1;
    const char * puid = _args->arg2;

    hb_grid_process_newmessageschannel_in_data_t in_data;
    if( hb_token_base16_decode_string( account_token, &in_data.token ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_base16_decode( puid, HB_UNKNOWN_STRING_SIZE, &in_data.puid, sizeof( in_data.puid ), HB_NULLPTR );

    in_data.maxpost = 256;

    hb_grid_process_newmessageschannel_out_data_t out_data;
    if( hb_grid_process_newmessageschannel( _process, &in_data, &out_data ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    size_t response_data_size = sprintf( _response, "{\"code\":0,\"id\":%d}"
        , out_data.muid
    );

    *_size = response_data_size;

    return HTTP_OK;
}