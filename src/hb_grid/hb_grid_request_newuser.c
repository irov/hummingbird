#include "hb_grid_request.h"

#include "hb_node_newuser/hb_node_newuser.h"

#include "hb_process/hb_process.h"
#include "hb_utils/hb_base64.h"

void hb_grid_request_newuser( struct evhttp_request * _request, void * _ud )
{
    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_newuser_in_t in_data;

    strcpy( in_data.db_uri, handle->db_uri );

    uint32_t multipart_params_count;
    multipart_params_handle_t multipart_params[8];
    if( hb_grid_get_request_params( _request, multipart_params, 8, &multipart_params_count ) == 0 )
    {
        return;
    }

    size_t params_puid_size;
    const void * params_puid;
    if( hb_multipart_get_value( multipart_params, multipart_params_count, "puid", &params_puid, &params_puid_size ) == 0 )
    {
        return;
    }
    
    size_t params_login_size;
    const void * params_login;
    if( hb_multipart_get_value( multipart_params, multipart_params_count, "login", &params_login, &params_login_size ) == 0 )
    {
        return;
    }

    size_t params_password_size;
    const void * params_password;
    if( hb_multipart_get_value( multipart_params, multipart_params_count, "password", &params_password, &params_password_size ) == 0 )
    {
        return;
    }
    
    hb_base64_decode( params_puid, params_puid_size, in_data.puid, 12, HB_NULLPTR );

    memcpy( in_data.login, params_login, params_login_size );
    in_data.login[params_login_size] = '\0';

    memcpy( in_data.password, params_password, params_password_size );
    in_data.password[params_password_size] = '\0';

    hb_sharedmemory_write( &handle->sharedmemory, &in_data, sizeof( in_data ) );

    char process_command[64];
    sprintf( process_command, "--sm %s"
        , handle->sharedmemory.name
    );

    int process_result = hb_process_run( "hb_node_newuser.exe", process_command );
    HB_UNUSED( process_result );

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_newuser_out_t out_data;
    hb_sharedmemory_read( &handle->sharedmemory, &out_data, sizeof( out_data ), HB_NULLPTR );

    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );

    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    if( out_data.exist == 0 )
    {
        size_t token64_size;
        char token64[25];
        hb_base64_encode( out_data.token, 12, token64, 25, &token64_size );
        token64[token64_size] = '\0';

        char request[256];
        size_t request_size = sprintf( request, "{\"token\"=\"%s\"}"
            , token64
        );

        evbuffer_add( output_buffer, request, request_size );
    }
    else
    {
        char request[256];
        size_t request_size = sprintf( request, "{\"error\"=\"1\"}" );

        evbuffer_add( output_buffer, request, request_size );
    }    

    evhttp_send_reply( _request, HTTP_OK, "", output_buffer );
}