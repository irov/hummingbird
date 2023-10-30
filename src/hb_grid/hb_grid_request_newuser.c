#include "hb_grid.h"

#include "hb_grid_process_newuser.h"
#include "hb_grid_process_script_api.h"

#include "hb_log/hb_log.h"
#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_newuser( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size )
{
    hb_bool_t required = HB_TRUE;

    const char * arg_project_uid;
    hb_json_get_field_string_required( _data, "project_uid", &arg_project_uid, HB_NULLPTR, &required );

    const char * arg_user_login;
    hb_json_get_field_string_required( _data, "user_login", &arg_user_login, HB_NULLPTR, &required );

    const char * arg_user_password;
    hb_json_get_field_string_required( _data, "user_password", &arg_user_password, HB_NULLPTR, &required );

    if( required == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_newuser_in_data_t in_data;

    if( hb_base16_decode( arg_project_uid, HB_UNKNOWN_STRING_SIZE, &in_data.project_uid, sizeof( in_data.project_uid ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    strncpy( in_data.user_login, arg_user_login, 128 );
    strncpy( in_data.user_password, arg_user_password, 128 );

    hb_grid_process_newuser_out_data_t out_data;
    if( hb_grid_process_newuser( _process, &in_data, &out_data ) == HB_FAILURE )
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
    
    api_in_data.json_args = HB_NULLPTR;

    strcpy( api_in_data.api, "event" );
    strcpy( api_in_data.method, "onCreateUser" );

    hb_grid_process_script_api_out_data_t api_out_data;
    if( hb_grid_process_script_api( _process, &api_in_data, &api_out_data ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( api_out_data.code != HB_ERROR_OK )
    {
        HB_LOG_MESSAGE_WARNING( "grid", "project '%u' user '%u' create user without script event '%s'"
            , api_in_data.project_uid
            , api_in_data.user_uid
            , api_in_data.method
        );
    }

    hb_size_t response_data_size = sprintf( _response, "{\"code\":0,\"uid\":%u,\"stat\":{\"memory_used\":%zu,\"call_used\":%u}}"
        , out_data.user_uid
        , api_out_data.memory_used
        , api_out_data.call_used
    );

    *_size = response_data_size;

    return HTTP_OK;
}