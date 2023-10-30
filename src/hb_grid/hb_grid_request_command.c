#include "hb_grid.h"
#include "hb_grid_process_script_api.h"

#include "hb_http/hb_http.h"
#include "hb_token/hb_token.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_command( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size )
{
    hb_bool_t required = HB_TRUE;

    const char * arg_account_token;
    hb_json_get_field_string_required( _data, "account_token", &arg_account_token, HB_NULLPTR, &required );

    const char * arg_project_uid;
    hb_json_get_field_string_required( _data, "project_uid", &arg_project_uid, HB_NULLPTR, &required );

    const char * arg_method;
    hb_json_get_field_string_required( _data, "method", &arg_method, HB_NULLPTR, &required );

    hb_json_handle_t * json_method_args;
    hb_json_get_field_required( _data, "args", &json_method_args, &required );

    if( required == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_account_token_t account_token;
    if( hb_cache_get_token( _process->cache, arg_account_token, 1800, &account_token, sizeof( hb_user_token_t ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_uid_t puid;
    if( hb_base16_decode( arg_project_uid, HB_UNKNOWN_STRING_SIZE, &puid, sizeof( hb_uid_t ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_script_api_in_data_t in_data;
    in_data.project_uid = puid;
    in_data.user_uid = HB_UID_NONE;

    strncpy( in_data.api, "command", 32 );
    strncpy( in_data.method, arg_method, 32 );

    in_data.json_args = json_method_args;

    hb_grid_process_lock( _process, account_token.account_uid );

    hb_grid_process_script_api_out_data_t out_data;
    hb_result_t result = hb_grid_process_script_api( _process, &in_data, &out_data );

    hb_grid_process_unlock( _process, account_token.account_uid );

    hb_json_destroy( json_method_args );

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

    hb_size_t response_data_size = sprintf( _response, "{\"code\":0,\"data\":%.*s,\"stat\":{\"memory_used\":%zu,\"call_used\":%u}}"
        , (int32_t)out_data.response_size
        , out_data.response_data
        , out_data.memory_used
        , out_data.call_used
    );

    *_size = response_data_size;    

    return HTTP_OK;
}