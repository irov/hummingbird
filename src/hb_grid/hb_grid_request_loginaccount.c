#include "hb_grid.h"

#include "hb_grid_process_loginaccount.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_loginaccount( hb_grid_request_handle_t * _args )
{    
    const char * arg_account_login;
    if( hb_json_get_field_string( _args->data, "account_login", &arg_account_login, HB_NULLPTR ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get account login" );

        return HTTP_BADREQUEST;
    }

    const char * arg_account_password;
    if( hb_json_get_field_string( _args->data, "account_password", &arg_account_password, HB_NULLPTR ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get account password" );

        return HTTP_BADREQUEST;
    }

    hb_grid_process_loginaccount_in_data_t in_data;
    strncpy( in_data.account_login, arg_account_login, 128 );
    strncpy( in_data.account_password, arg_account_password, 128 );

    hb_grid_process_loginaccount_out_data_t out_data;
    if( hb_grid_process_loginaccount( _args->process, &in_data, &out_data ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( out_data.exist == HB_TRUE )
    {
        hb_token16_t token16;
        if( hb_token_base16_encode( &out_data.token, &token16 ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":0,\"token\":\"%.*s\"}"
            , (int32_t)sizeof( token16 )
            , token16.value
        );
    }
    else
    {
        snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":1,\"reason\":\"invalid login or password\"}" );
    }

    return HTTP_OK;
}