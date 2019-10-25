#include "hb_grid_request.h"

#include "hb_node_newuser/hb_node_newuser.h"

#include "hb_process/hb_process.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

void hb_grid_request_newuser( struct evhttp_request * _request, void * _ud )
{
    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );

    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_newuser_in_t in_data;
    in_data.magic_number = hb_node_newuser_magic_number;
    in_data.version_number = hb_node_newuser_version_number;

    strcpy( in_data.cache_uri, handle->cache_uri );
    in_data.cache_port = handle->cache_port;

    strcpy( in_data.db_uri, handle->db_uri );

    uint32_t multipart_params_count;
    multipart_params_handle_t multipart_params[8];
    if( hb_grid_get_request_params( _request, multipart_params, 8, &multipart_params_count ) == 0 )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    size_t params_pid_size;
    const void * params_pid;
    if( hb_multipart_get_value( multipart_params, multipart_params_count, "pid", &params_pid, &params_pid_size ) == 0 )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }
    
    size_t params_login_size;
    const void * params_login;
    if( hb_multipart_get_value( multipart_params, multipart_params_count, "login", &params_login, &params_login_size ) == 0 )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    size_t params_password_size;
    const void * params_password;
    if( hb_multipart_get_value( multipart_params, multipart_params_count, "password", &params_password, &params_password_size ) == 0 )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }
    
    hb_base16_decode( params_pid, params_pid_size, &in_data.pid, 2, HB_NULLPTR );

    memcpy( in_data.login, params_login, params_login_size );
    in_data.login[params_login_size] = '\0';

    memcpy( in_data.password, params_password, params_password_size );
    in_data.password[params_password_size] = '\0';

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

        char request_data[HB_GRID_REQUEST_MAX_SIZE];
        size_t request_data_size = sprintf( request_data, "{\"token\"=\"%.24s\"}"
            , token16
        );

        evbuffer_add( output_buffer, request_data, request_data_size );
    }
    else
    {
        char request_data[HB_GRID_REQUEST_MAX_SIZE];
        size_t request_data_size = sprintf( request_data, "{\"error\"=\"1\"}" );

        evbuffer_add( output_buffer, request_data, request_data_size );
    }    

    evhttp_send_reply( _request, HTTP_OK, "", output_buffer );
}