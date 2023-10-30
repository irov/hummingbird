#include "hb_grid.h"

#include "hb_grid_process_newproject.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

hb_http_code_t hb_grid_request_newproject( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size )
{
    hb_bool_t required = HB_TRUE;

    const char * arg_account_token;
    hb_json_get_field_string_required( _data, "account_token", &arg_account_token, HB_NULLPTR, &required );

    if( required == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_account_token_t account_token;
    if( hb_cache_get_token( _process->cache, arg_account_token, 1800, &account_token, sizeof( account_token ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_newproject_in_data_t in_data;
    in_data.account_uid = account_token.account_uid;

    hb_grid_process_lock( _process, account_token.account_uid );

    hb_grid_process_newproject_out_data_t out_data;
    hb_result_t result = hb_grid_process_newproject( _process, &in_data, &out_data );

    hb_grid_process_unlock( _process, account_token.account_uid );

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
    if( hb_base16_encode( &out_data.project_uid, sizeof( out_data.project_uid ), uid16.value, sizeof( uid16 ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_size_t response_data_size = sprintf( _response, "{\"code\":0,\"pid\":\"%.*s\"}"
        , (int32_t)sizeof( uid16 )
        , uid16.value
    );

    *_size = response_data_size;

    return HTTP_OK;
}