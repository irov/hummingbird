#include "hb_grid.h"
#include "hb_grid_process_newaccount.h"

#include "hb_db/hb_db.h"
#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"
#include "hb_utils/hb_sha1.h"

#include <string.h>

hb_http_code_t hb_grid_request_newaccount( hb_grid_request_handle_t * _args )
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

    hb_grid_process_newaccount_in_data_t in_data;
    strncpy( in_data.account_login, arg_account_login, 128 );
    strncpy( in_data.account_password, arg_account_password, 128 );

    hb_grid_process_newaccount_out_data_t out_data;
    if( hb_grid_process_newaccount( _args->process, &in_data, &out_data ) == HB_FAILURE )
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

    snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":0}" );

    return HTTP_OK;
}