#include "hb_grid_request.h"

#include "hb_node_loginuser/hb_node_loginuser.h"

#include "hb_token/hb_token.h"
#include "hb_process/hb_process.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base16.h"

void hb_grid_request_loginuser( struct evhttp_request * _request, void * _ud )
{
    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );

    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

    hb_node_loginuser_in_t in_data;

    size_t request_data_size;
    char request_data[HB_GRID_REQUEST_DATA_MAX_SIZE];
    if( hb_grid_get_request_data( _request, request_data, HB_GRID_REQUEST_DATA_MAX_SIZE, &request_data_size ) == 0 )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    hb_json_handle_t json_handle;
    if( hb_json_create( request_data, request_data_size, &json_handle ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    const char * pid;
    if( hb_json_get_field_string( &json_handle, "pid", &pid, HB_NULLPTR ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    const char * login;
    if( hb_json_get_field_string( &json_handle, "login", &login, HB_NULLPTR ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    const char * password;
    if( hb_json_get_field_string( &json_handle, "password", &password, HB_NULLPTR ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    hb_base16_decode( pid, ~0U, &in_data.pid, sizeof( in_data.pid ), HB_NULLPTR );

    strcpy( in_data.login, login );
    strcpy( in_data.password, password );

    if( hb_node_write_in_data( &handle->sharedmemory, &in_data, sizeof( in_data ), &handle->config ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    hb_bool_t process_successful;
    if( hb_process_run( "hb_node_loginuser.exe", handle->sharedmemory.name, &process_successful ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    if( process_successful == HB_FALSE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    hb_node_loginuser_out_t out_data;
    hb_node_code_t out_code;
    if( hb_node_read_out_data( &handle->sharedmemory, &out_data, sizeof( out_data ), &out_code ) == HB_FAILURE)
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    if( out_code != e_node_ok )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    if( out_data.exist == HB_TRUE )
    {
        hb_token16_t token16;
        hb_token_base16_encode( out_data.token, token16 );

        char response_data[HB_GRID_REQUEST_DATA_MAX_SIZE];
        size_t response_data_size = sprintf( response_data, "{\"code\": 0, \"token\": \"%.*s\"}"
            , sizeof( token16 )
            , token16
        );

        evbuffer_add( output_buffer, response_data, response_data_size );
    }
    else
    {
        char response_data[HB_GRID_REQUEST_DATA_MAX_SIZE];
        size_t response_data_size = sprintf( response_data, "{\"code\": 1}" );

        evbuffer_add( output_buffer, response_data, response_data_size );
    }    

    evhttp_send_reply( _request, HTTP_OK, "", output_buffer );
}