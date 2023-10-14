#include "hb_grid.h"

#include "hb_grid_process_loginuser.h"
#include "hb_grid_process_script_api.h"

#include "hb_log/hb_log.h"
#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_loginuser( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, hb_size_t * _size, const hb_grid_process_cmd_args_t * _args )
{
    const char * arg_puid = _args->arg1;

    hb_bool_t required_successful = HB_TRUE;

    hb_grid_process_loginuser_in_data_t in_data;

    if( hb_base16_decode( arg_puid, HB_UNKNOWN_STRING_SIZE, &in_data.project_uid, sizeof( in_data.project_uid ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    {
        hb_json_handle_t * json_handle;
        if( hb_http_get_request_json( _request, &json_handle ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_copy_field_string_required( json_handle, "login", in_data.login, 128, &required_successful ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_copy_field_string_required( json_handle, "password", in_data.password, 128, &required_successful ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_get_field_int32( json_handle, "user_public_data_revision", &in_data.user_public_data_revision, -1 ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_get_field_int32( json_handle, "project_public_data_revision", &in_data.project_public_data_revision, -1 ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        hb_json_destroy( json_handle );
    }

    if( required_successful == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_loginuser_out_data_t out_data;
    if( hb_grid_process_loginuser( _process, &in_data, &out_data ) == HB_FAILURE )
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

    hb_grid_process_script_api_in_data_t api_in_data;

    api_in_data.project_uid = in_data.project_uid;
    api_in_data.user_uid = out_data.user_uid;

    api_in_data.json_handle = HB_NULLPTR;

    strcpy( api_in_data.api, "event" );
    strcpy( api_in_data.method, "onLoginUser" );

    hb_grid_process_script_api_out_data_t api_out_data;
    if( hb_grid_process_script_api( _process, &api_in_data, &api_out_data ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( api_out_data.code != HB_ERROR_OK )
    {
        HB_LOG_MESSAGE_WARNING( "grid", "project '%u' user '%u' login user without script event '%s'"
            , api_in_data.project_uid
            , api_in_data.user_uid
            , api_in_data.method
        );
    }

    hb_token16_t token16;
    if( hb_token_base16_encode( &out_data.token, &token16 ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_size_t response_data_size = sprintf( _response, "{\"code\":0,\"uid\":%u,\"token\":\"%.*s\", \"user_data_revision\":%d,\"user_data\":%s,\"project_data_revision\":%u,\"project_data\":%s,\"stat\":{\"memory_used\":%zu,\"call_used\":%u}}"
        , out_data.user_uid
        , (int32_t)sizeof( token16 )
        , token16.value
        , in_data.user_public_data_revision != out_data.user_public_data_revision ? out_data.user_public_data_revision : -1
        , in_data.user_public_data_revision != out_data.user_public_data_revision ? out_data.user_public_data : "{}"
        , in_data.project_public_data_revision != out_data.project_public_data_revision ? out_data.project_public_data_revision : -1
        , in_data.project_public_data_revision != out_data.project_public_data_revision ? out_data.project_public_data : "{}"
        , api_out_data.memory_used
        , api_out_data.call_used
    );

    *_size = response_data_size;

    return HTTP_OK;
}