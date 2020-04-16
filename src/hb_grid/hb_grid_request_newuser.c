#include "hb_grid.h"

#include "hb_grid_process_newuser.h"
#include "hb_grid_process_script_event.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

int hb_grid_request_newuser( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const char * _pid )
{
    hb_grid_process_newuser_in_data_t in_data;

    hb_base16_decode( _pid, HB_UNKNOWN_STRING_SIZE, &in_data.pid, sizeof( in_data.pid ), HB_NULLPTR );

    {
        hb_json_handle_t * json_handle;
        if( hb_http_get_request_json( _request, &json_handle ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_copy_field_string( json_handle, "login", in_data.login, 128 ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_copy_field_string( json_handle, "password", in_data.password, 128 ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        hb_json_destroy( json_handle );
    }

    hb_grid_process_newuser_out_data_t out_data;
    if( hb_grid_process_newuser( _process, &in_data, &out_data ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( out_data.exist == HB_FALSE )
    {
        hb_grid_process_api_in_data_t api_in_data;

        api_in_data.data_size = 0;

        hb_token_copy( api_in_data.token, out_data.token );

        api_in_data.category = e_hb_node_event;
        strcpy( api_in_data.method, "onCreateUser" );

        hb_grid_process_api_out_data_t api_out_data;
        if( hb_grid_process_api( _process, &api_in_data, &api_out_data ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( api_out_data.successful == HB_FALSE && api_out_data.method_found == HB_TRUE )
        {
            size_t response_data_size = sprintf( _response, "{\"code\": 1, \"reason\": \"error event onCreatUser\"}" );

            *_size = response_data_size;

            return HTTP_OK;
        }

        hb_token16_t token16;
        if( hb_token_base16_encode( out_data.token, &token16 ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        size_t response_data_size = sprintf( _response, "{\"code\": 0, \"token\": \"%.*s\", \"stat\": {\"memory_used\": %zu, \"call_used\": %u}}"
            , (int)sizeof( token16 )
            , token16
            , api_out_data.memory_used
            , api_out_data.call_used
        );

        *_size = response_data_size;
    }
    else
    {
        size_t response_data_size = sprintf( _response, "{\"code\": 1, \"reason\": \"already exist\"}" );

        *_size = response_data_size;
    }

    return HTTP_OK;
}