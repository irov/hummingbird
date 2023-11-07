#include "hb_grid.h"

#include "hb_grid_process_newproject.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

hb_http_code_t hb_grid_request_newproject( hb_grid_request_handle_t * _args )
{
    const char * arg_account_token;
    if( hb_json_get_field_string( _args->data, "account_token", &arg_account_token, HB_NULLPTR ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get account token" );

        return HTTP_BADREQUEST;
    }

    hb_account_token_t account_token;
    if( hb_cache_get_token( _args->process->cache, arg_account_token, 1800, &account_token, sizeof( account_token ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_newproject_in_data_t in_data;
    in_data.account_uid = account_token.account_uid;

    hb_grid_process_lock( _args->process, account_token.account_uid );

    hb_grid_process_newproject_out_data_t out_data;
    hb_result_t result = hb_grid_process_newproject( _args->process, &in_data, &out_data );

    hb_grid_process_unlock( _args->process, account_token.account_uid );

    if( result == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( out_data.code != HB_ERROR_OK )
    {
        snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":%u}"
            , out_data.code
        );

        return HTTP_OK;
    }

    hb_uid16_t uid16;
    if( hb_base16_encode( &out_data.project_uid, sizeof( out_data.project_uid ), uid16.value, sizeof( uid16 ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":0,\"pid\":\"%.*s\"}"
        , (int32_t)sizeof( uid16 )
        , uid16.value
    );

    return HTTP_OK;
}