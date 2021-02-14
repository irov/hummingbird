#include "hb_grid.h"

#include "hb_grid_process_setusernickname.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_setusernickname( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, hb_size_t * _size, const hb_grid_process_cmd_args_t * _args )
{
    const char * arg_user_token = _args->arg1;

    hb_user_token_t token_handle;
    if( hb_cache_get_token( _process->cache, arg_user_token, 1800, &token_handle, sizeof( token_handle ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_bool_t required_successful = HB_TRUE;

    hb_grid_process_setusernickname_in_data_t in_data;
    in_data.uuid = token_handle.uuid;
    in_data.puid = token_handle.puid;

    {
        hb_json_handle_t * json_handle;
        if( hb_http_get_request_json( _request, &json_handle ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_copy_field_string_required( json_handle, "nickname", in_data.nickname, 32, &required_successful ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        hb_json_destroy( json_handle );
    }

    if( required_successful == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_mutex_handle_t * mutex_handle = _process->mutex_handles + token_handle.uuid % _process->mutex_count;
    hb_mutex_lock( mutex_handle->mutex );

    hb_grid_process_setusernickname_out_data_t out_data;
    hb_result_t result = hb_grid_process_setusernickname( _process, &in_data, &out_data );

    hb_mutex_unlock( mutex_handle->mutex );

    if( result == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_size_t response_data_size = sprintf( _response, "{\"code\":0}" );

    *_size = response_data_size;    

    return HTTP_OK;
}