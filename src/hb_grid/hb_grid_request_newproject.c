#include "hb_grid.h"

#include "hb_grid_process_newproject.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

hb_http_code_t hb_grid_request_newproject( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, hb_size_t * _size, const hb_grid_process_cmd_args_t * _args )
{
    HB_UNUSED( _request );

    const char * arg_account_token = _args->arg1;

    hb_account_token_t token_handle;
    if( hb_cache_get_token( _process->cache, arg_account_token, 1800, &token_handle, sizeof( token_handle ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_grid_process_newproject_in_data_t in_data;
    in_data.auid = token_handle.auid;

    hb_grid_mutex_handle_t * mutex_handle = _process->mutex_handles + token_handle.auid % _process->mutex_count;
    hb_mutex_lock( mutex_handle->mutex );

    hb_grid_process_newproject_out_data_t out_data;
    hb_result_t result = hb_grid_process_newproject( _process, &in_data, &out_data );

    hb_mutex_unlock( mutex_handle->mutex );

    if( result == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( out_data.code != HB_ERROR_OK )
    {
        hb_size_t response_data_size = sprintf( _response, "{\"code\":%u}"
            , out_data.code
        );

        *_size = response_data_size;

        return HTTP_OK;
    }

    hb_uid16_t uid16;
    if( hb_base16_encode( &out_data.puid, sizeof( out_data.puid ), uid16.value, sizeof( uid16 ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_size_t response_data_size = sprintf( _response, "{\"code\":0,\"pid\":\"%.*s\"}"
        , (int)sizeof( uid16 )
        , uid16.value
    );

    *_size = response_data_size;

    return HTTP_OK;
}