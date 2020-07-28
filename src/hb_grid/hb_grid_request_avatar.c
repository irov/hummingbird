#include "hb_grid.h"
#include "hb_grid_process_script_api.h"

#include "hb_http/hb_http.h"
#include "hb_token/hb_token.h"
#include "hb_utils/hb_base64.h"

#include <string.h>

hb_http_code_t hb_grid_request_avatar( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _args )
{
    const char * arg_user_token = _args->arg1;
    const char * arg_method = _args->arg2;

    hb_user_token_t token_handle;
    if( hb_cache_get_token( _process->cache, arg_user_token, 1800, &token_handle, sizeof( hb_user_token_t ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_script_api_in_data_t in_data;
    in_data.puid = token_handle.puid;
    in_data.uuid = token_handle.uuid;
    strcpy( in_data.api, "avatar" );
    strcpy( in_data.method, arg_method );

    if( hb_http_get_request_json( _request, &in_data.json_handle ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_script_api_out_data_t out_data;
    if( hb_grid_process_script_api( _process, &in_data, &out_data ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_json_destroy( in_data.json_handle );

    if( out_data.code != HB_ERROR_OK )
    {
        size_t response_data_size = sprintf( _response, "{\"code\":%u}"
            , out_data.code
        );

        *_size = response_data_size;

        return HTTP_OK;
    }

    size_t response_data_size = sprintf( _response, "{\"code\":0,\"data\":%.*s,\"stat\":{\"memory_used\":%zu,\"call_used\":%u}}"
        , (int)out_data.response_size
        , out_data.response_data
        , out_data.memory_used
        , out_data.call_used
    );

    *_size = response_data_size;

    return HTTP_OK;
}