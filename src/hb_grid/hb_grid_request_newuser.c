#include "hb_grid_request.h"

#include "hb_node_newuser/hb_node_newuser.h"
#include "hb_node_api/hb_node_api.h"

#include "hb_token/hb_token.h"
#include "hb_process/hb_process.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

void hb_grid_request_newuser( struct evhttp_request * _request, void * _ud )
{
    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );

    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

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

    hb_node_newuser_out_t out_data;

    {
        hb_node_newuser_in_t in_data;

        strcpy( in_data.cache_uri, handle->cache_uri );
        in_data.cache_port = handle->cache_port;

        strcpy( in_data.db_uri, handle->db_uri );

        hb_base16_decode( pid, ~0U, &in_data.pid, sizeof( in_data.pid ), HB_NULLPTR );

        strcpy( in_data.login, login );
        strcpy( in_data.password, password );

        if( hb_node_write_in_data( &handle->sharedmemory, &in_data, sizeof( in_data ), hb_node_newuser_magic_number, hb_node_newuser_version_number ) == HB_FAILURE )
        {
            evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

            return;
        }

        hb_bool_t process_successful;
        if( hb_process_run( "hb_node_newuser.exe", handle->sharedmemory.name, &process_successful ) == HB_FAILURE )
        {
            evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

            return;
        }

        if( process_successful == HB_FALSE )
        {
            evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

            return;
        }

        uint32_t out_code;
        if( hb_node_read_out_data( &handle->sharedmemory, &out_data, sizeof( out_data ), hb_node_newuser_magic_number, hb_node_newuser_version_number, &out_code ) == HB_FAILURE )
        {
            evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

            return;
        }

        if( out_code != 0 )
        {
            evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

            return;
        }
    }

    {
        hb_node_api_in_t api_in_data;

        strcpy( api_in_data.cache_uri, handle->cache_uri );
        api_in_data.cache_port = handle->cache_port;

        strcpy( api_in_data.db_uri, handle->db_uri );

        api_in_data.data_size = 0;

        hb_token_copy( api_in_data.token, out_data.token );

        api_in_data.category = e_hb_node_event;
        strcpy( api_in_data.method, "onCreateUser" );

        hb_node_write_in_data( &handle->sharedmemory, &api_in_data, sizeof( api_in_data ), hb_node_api_magic_number, hb_node_api_version_number );

        hb_bool_t process_successful;
        if( hb_process_run( "hb_node_api.exe", handle->sharedmemory.name, &process_successful ) == HB_FAILURE )
        {
            evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

            return;
        }

        if( process_successful == HB_FALSE )
        {
            evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

            return;
        }

        hb_node_api_out_t api_out_data;
        uint32_t out_code;
        if( hb_node_read_out_data( &handle->sharedmemory, &api_out_data, sizeof( api_out_data ), hb_node_api_magic_number, hb_node_api_version_number, &out_code ) == HB_FAILURE )
        {
            evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

            return;
        }

        if( out_code != 0 )
        {
            evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

            return;
        }
    }

    if( out_data.exist == 0 )
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