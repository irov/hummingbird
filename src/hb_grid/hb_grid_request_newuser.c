#include "hb_grid.h"

#include "hb_grid_process_newuser.h"
#include "hb_grid_process_script_event.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_newuser( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _args )
{
    const char * puid = _args->arg1;

    hb_bool_t required_successful = HB_TRUE;

    hb_grid_process_newuser_in_data_t in_data;

    if( hb_base16_decode( puid, HB_UNKNOWN_STRING_SIZE, &in_data.puid, sizeof( in_data.puid ), HB_NULLPTR ) == HB_FAILURE )
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

        hb_json_destroy( json_handle );
    }

    if( required_successful == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_newuser_out_data_t out_data;
    if( hb_grid_process_newuser( _process, &in_data, &out_data ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( out_data.code != HB_ERROR_OK )
    {
        size_t response_data_size = sprintf( _response, "{\"code\":%u}"
            , out_data.code
        );

        *_size = response_data_size;

        return HTTP_OK;
    }

    hb_grid_process_api_in_data_t api_in_data;

    api_in_data.data_size = 0;

    hb_token_copy( &api_in_data.token, &out_data.token );

    api_in_data.category = e_hb_node_event;
    strcpy( api_in_data.method, "onCreateUser" );

    hb_grid_process_api_out_data_t api_out_data;
    if( hb_grid_process_api( _process, &api_in_data, &api_out_data ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( api_out_data.successful == HB_FALSE && api_out_data.method_found == HB_TRUE )
    {
        size_t response_data_size = sprintf( _response, "{\"code\":2}" );

        *_size = response_data_size;

        return HTTP_OK;
    }

    hb_token16_t token16;
    if( hb_token_base16_encode( &out_data.token, &token16 ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    size_t response_data_size = sprintf( _response, "{\"code\":0,\"uid\":%u,\"token\":\"%.*s\",\"stat\":{\"memory_used\":%zu,\"call_used\":%u}}"
        , out_data.uuid
        , (int)sizeof( token16 )
        , token16.value
        , api_out_data.memory_used
        , api_out_data.call_used
    );

    *_size = response_data_size;

    return HTTP_OK;
}