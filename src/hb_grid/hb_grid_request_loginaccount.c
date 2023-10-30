#include "hb_grid.h"

#include "hb_grid_process_loginaccount.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
hb_http_code_t hb_grid_request_loginaccount( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size )
{
    hb_bool_t required = HB_TRUE;

    const char * arg_account_login;
    hb_json_get_field_string_required( _data, "account_login", &arg_account_login, HB_NULLPTR, &required );

    const char * arg_account_password;
    hb_json_get_field_string_required( _data, "account_password", &arg_account_password, HB_NULLPTR, &required );

    if( required == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_loginaccount_in_data_t in_data;
    strncpy( in_data.account_login, arg_account_login, 128 );
    strncpy( in_data.account_password, arg_account_password, 128 );

    hb_grid_process_loginaccount_out_data_t out_data;
    if( hb_grid_process_loginaccount( _process, &in_data, &out_data ) == HB_FAILURE )
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

        hb_size_t response_data_size = sprintf( _response, "{\"code\":0,\"token\":\"%.*s\"}"
            , (int32_t)sizeof( token16 )
            , token16.value
        );

        *_size = response_data_size;
    }
    else
    {
        hb_size_t response_data_size = sprintf( _response, "{\"code\":1,\"reason\":\"invalid login or password\"}" );

        *_size = response_data_size;
    }

    return HTTP_OK;
}