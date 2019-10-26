#include "hb_grid_request.h"

#include "hb_node_newuser/hb_node_newuser.h"

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

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_newuser_in_t in_data;
    in_data.magic_number = hb_node_newuser_magic_number;
    in_data.version_number = hb_node_newuser_version_number;

    strcpy( in_data.cache_uri, handle->cache_uri );
    in_data.cache_port = handle->cache_port;

    strcpy( in_data.db_uri, handle->db_uri );

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
    if( hb_json_get_string( &json_handle, "pid", &pid, HB_NULLPTR ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }
    
    const char * login;
    if( hb_json_get_string( &json_handle, "login", &login, HB_NULLPTR ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    const char * password;
    if( hb_json_get_string( &json_handle, "password", &password, HB_NULLPTR ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }
    
    hb_base16_decode( pid, ~0U, &in_data.pid, 2, HB_NULLPTR );

    strcpy( in_data.login, login );
    strcpy( in_data.password, password );

    hb_sharedmemory_write( &handle->sharedmemory, &in_data, sizeof( in_data ) );

    char process_command[64];
    sprintf( process_command, "--sm %s"
        , handle->sharedmemory.name
    );

    hb_result_t process_result = hb_process_run( "hb_node_newuser.exe", process_command );
    HB_UNUSED( process_result );

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_newuser_out_t out_data;
    hb_sharedmemory_read( &handle->sharedmemory, &out_data, sizeof( out_data ), HB_NULLPTR );

    if( out_data.magic_number != hb_node_newuser_magic_number )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    if( out_data.version_number != hb_node_newuser_version_number )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    if( out_data.exist == 0 )
    {
        char token16[24];
        hb_base16_encode( out_data.token, 12, token16, 24, HB_NULLPTR );

        char response_data[HB_GRID_REQUEST_DATA_MAX_SIZE];
        size_t response_data_size = sprintf( response_data, "{\"code\": 0, \"token\": \"%.24s\"}"
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