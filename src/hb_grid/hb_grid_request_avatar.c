#include "hb_grid.h"
#include "hb_grid_process_api.h"

#include "hb_http/hb_http.h"
#include "hb_token/hb_token.h"
#include "hb_utils/hb_base64.h"

#include <string.h>

hb_http_code_t hb_grid_request_avatar( hb_grid_request_handle_t * _args )
{
    const char * arg_user_token;
    if( hb_json_get_field_string( _args->data, "user_token", &arg_user_token, HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    const char * arg_method;
    if( hb_json_get_field_string( _args->data, "method", &arg_method, HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_json_handle_t * json_method_args;
    if( hb_json_get_field( _args->data, "args", &json_method_args ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }    

    hb_user_token_t user_token;
    if( hb_cache_get_token( _args->process->cache, arg_user_token, 1800, &user_token, sizeof( hb_user_token_t ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_api_in_data_t in_data;
    in_data.project_uid = user_token.project_uid;
    in_data.user_uid = user_token.user_uid;
    strcpy( in_data.api, "avatar" );
    strcpy( in_data.method, arg_method );
    in_data.json_args = json_method_args;

    hb_grid_process_lock( _args->process, user_token.user_uid );

    hb_grid_process_api_out_data_t out_data;
    hb_result_t result = hb_grid_process_api( _args->process, &in_data, &out_data );

    hb_grid_process_unlock( _args->process, user_token.user_uid );

    hb_json_destroy( in_data.json_args );

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

    snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":0,\"data\":%.*s,\"stat\":{\"memory_used\":%zu,\"call_used\":%u}}"
        , (int32_t)out_data.response_size
        , out_data.response_data
        , out_data.memory_used
        , out_data.call_used
    );

    return HTTP_OK;
}