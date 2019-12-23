#include "hb_grid.h"

#include "hb_node_loginaccount/hb_node_loginaccount.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_process/hb_process.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

int hb_grid_request_loginaccount( struct evhttp_request * _request, struct hb_grid_process_handle_t * _handle, char * _response, size_t * _size )
{
    hb_node_loginaccount_in_t in_data;

    {
        hb_json_handle_t * json_handle;
        if( hb_http_get_request_json( _request, &json_handle ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        const char * login;
        if( hb_json_get_field_string( json_handle, "login", &login, HB_NULLPTR, HB_NULLPTR ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        const char * password;
        if( hb_json_get_field_string( json_handle, "password", &password, HB_NULLPTR, HB_NULLPTR ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }        

        strcpy( in_data.login, login );
        strcpy( in_data.password, password );

        hb_json_destroy( json_handle );
    }

    if( hb_node_write_in_data( _handle->sharedmemory, &in_data, sizeof( in_data ), _handle->config ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_bool_t process_successful;
    if( hb_process_run( _handle->config->process_loginaccount, _handle->sharedmemory, &process_successful ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( process_successful == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_node_loginaccount_out_t out_data;
    hb_node_code_t out_code;
    char out_reason[1024];
    if( hb_node_read_out_data( _handle->sharedmemory, &out_data, sizeof( out_data ), &out_code, out_reason ) == HB_FAILURE)
    {
        return HTTP_BADREQUEST;
    }

    if( out_code != e_node_ok )
    {
        size_t response_data_size = sprintf( _response, "{\"code\": 1, \"reason\": \"%s\"}"
            , out_reason
        );

        *_size = response_data_size;

        return HTTP_OK;
    }

    if( out_data.exist == HB_TRUE )
    {
        hb_token16_t token16;
        hb_token_base16_encode( out_data.token, token16 );

        size_t response_data_size = sprintf( _response, "{\"code\": 0, \"token\": \"%.*s\"}"
            , (int)sizeof( token16 )
            , token16
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