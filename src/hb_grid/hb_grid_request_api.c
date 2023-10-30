#include "hb_grid.h"
#include "hb_grid_process_script_api.h"

#include "hb_http/hb_http.h"
#include "hb_token/hb_token.h"
#include "hb_utils/hb_base64.h"

#include <string.h>

hb_http_code_t hb_grid_request_api( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size )
{
    hb_bool_t required = HB_TRUE;

    const char * arg_user_token;
    hb_json_get_field_string_required( _data, "user_token", &arg_user_token, HB_NULLPTR, &required );

    const char * arg_method;
    hb_json_get_field_string_required( _data, "method", &arg_method, HB_NULLPTR, &required );

    hb_json_handle_t * json_method_args;
    hb_json_get_field_required( _data, "args", &json_method_args, &required );

    if( required == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_user_token_t user_token;
    if( hb_cache_get_token( _process->cache, arg_user_token, 1800, &user_token, sizeof( hb_user_token_t ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_script_api_in_data_t in_data;
    in_data.project_uid = user_token.project_uid;
    in_data.user_uid = user_token.user_uid;
    strcpy( in_data.api, "api" );
    strcpy( in_data.method, arg_method );
    in_data.json_args = json_method_args;

    hb_grid_process_lock( _process, user_token.user_uid );

    hb_grid_process_script_api_out_data_t out_data;
    hb_result_t result = hb_grid_process_script_api( _process, &in_data, &out_data );

    hb_grid_process_unlock( _process, user_token.user_uid );

    hb_json_destroy( in_data.json_args );

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

    hb_size_t response_data_size = snprintf( _response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":0,\"data\":%.*s,\"stat\":{\"memory_used\":%zu,\"call_used\":%u}}"
        , (int32_t)out_data.response_size
        , out_data.response_data
        , out_data.memory_used
        , out_data.call_used
    );

    *_size = response_data_size;

    return HTTP_OK;
}