#include "hb_grid.h"

#include "hb_grid_process_loginuser.h"
#include "hb_grid_process_api.h"

#include "hb_log/hb_log.h"
#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_loginuser( hb_grid_request_handle_t * _args )
{    
    const char * arg_project_uid;
    if( hb_json_get_field_string( _args->data, "project_uid", &arg_project_uid, HB_NULLPTR ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get project uid" );

        return HTTP_BADREQUEST;
    }

    const char * arg_user_login;
    if( hb_json_get_field_string( _args->data, "user_login", &arg_user_login, HB_NULLPTR ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get user login" );

        return HTTP_BADREQUEST;
    }

    const char * arg_user_password;
    if( hb_json_get_field_string( _args->data, "user_password", &arg_user_password, HB_NULLPTR ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get user password" );

        return HTTP_BADREQUEST;
    }

    int32_t arg_user_public_data_revision;
    if( hb_json_get_field_int32( _args->data, "user_public_data_revision", &arg_user_public_data_revision ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get user public data revision" );

        return HTTP_BADREQUEST;
    }

    int32_t arg_project_public_data_revision;
    if( hb_json_get_field_int32( _args->data, "project_public_data_revision", &arg_project_public_data_revision ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get project public data revision" );

        return HTTP_BADREQUEST;
    }

    hb_grid_process_loginuser_in_data_t in_data;

    if( hb_base16_decode( arg_project_uid, HB_UNKNOWN_STRING_SIZE, &in_data.project_uid, sizeof( in_data.project_uid ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    strncpy( in_data.login, arg_user_login, 128 );
    strncpy( in_data.password, arg_user_password, 128 );

    in_data.user_public_data_revision = arg_user_public_data_revision;
    in_data.project_public_data_revision = arg_project_public_data_revision;

    hb_grid_process_loginuser_out_data_t out_data;
    if( hb_grid_process_loginuser( _args->process, &in_data, &out_data ) == HB_FAILURE )
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

    hb_grid_process_api_in_data_t api_in_data;

    api_in_data.project_uid = in_data.project_uid;
    api_in_data.user_uid = out_data.user_uid;

    api_in_data.json_args = HB_NULLPTR;

    strcpy( api_in_data.api, "event" );
    strcpy( api_in_data.method, "onLoginUser" );

    hb_grid_process_api_out_data_t api_out_data;
    if( hb_grid_process_api( _args->process, &api_in_data, &api_out_data ) == HB_FAILURE )
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

    sprintf( _args->response, "{\"code\":0,\"uid\":%u,\"token\":\"%.*s\",\"user_data_revision\":%d,\"user_data\":%s,\"project_data_revision\":%u,\"project_data\":%s,\"stat\":{\"memory_used\":%zu,\"call_used\":%u}}"
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

    return HTTP_OK;
}