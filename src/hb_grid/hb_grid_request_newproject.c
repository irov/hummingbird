#include "hb_grid_request.h"

#include "hb_node_newproject/hb_node_newproject.h"

#include "hb_process/hb_process.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

void hb_grid_request_newproject( struct evhttp_request * _request, void * _ud )
{
    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );

    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

    hb_node_newproject_in_t in_data;

    strcpy( in_data.db_uri, handle->db_uri );

    if( hb_node_write_in_data( &handle->sharedmemory, &in_data, sizeof( in_data ), hb_node_newproject_magic_number, hb_node_newproject_version_number ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    char process_command[64];
    sprintf( process_command, "--sm %s"
        , handle->sharedmemory.name
    );

    hb_bool_t process_successful;
    if( hb_process_run( "hb_node_newproject.exe", process_command, &process_successful ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    if( process_successful == HB_FALSE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    hb_node_newproject_out_t out_data;
    if( hb_node_read_out_data( &handle->sharedmemory, &out_data, sizeof( out_data ), hb_node_newproject_magic_number, hb_node_newproject_version_number ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    hb_pid16_t pid16;
    hb_base16_encode( &out_data.pid, sizeof( out_data.pid ), pid16, sizeof( pid16 ), HB_NULLPTR );

    char response_data[HB_GRID_REQUEST_DATA_MAX_SIZE];
    size_t response_data_size = sprintf( response_data, "{\"code\": 0, \"pid\": \"%.*s\"}"
        , sizeof( pid16 )
        , pid16
    );

    evbuffer_add( output_buffer, response_data, response_data_size );

    evhttp_send_reply( _request, HTTP_OK, "", output_buffer );
}