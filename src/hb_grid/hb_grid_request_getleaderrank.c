#include "hb_grid.h"

#include "hb_grid_process_getleaderrank.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_getleaderrank( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, hb_size_t * _size, const hb_grid_process_cmd_args_t * _args )
{
    HB_UNUSED( _request );

    const char * arg_user_token = _args->arg1;

    hb_user_token_t user_token;
    if( hb_cache_get_token( _process->cache, arg_user_token, 1800, &user_token, sizeof( user_token ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_bool_t required_successful = HB_TRUE;

    hb_grid_process_getleaderrank_in_data_t in_data;
    in_data.puid = user_token.puid;
    in_data.uuid = user_token.uuid;

    if( required_successful == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_lock( _process, user_token.uuid );

    hb_grid_process_getleaderrank_out_data_t out_data;
    hb_result_t result = hb_grid_process_getleaderrank( _process, &in_data, &out_data );

    hb_grid_process_unlock( _process, user_token.uuid );

    if( result == HB_FAILURE )
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