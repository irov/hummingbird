#include "hb_grid_request.h"

#include "hb_node_newproject/hb_node_newproject.h"

#include "hb_process/hb_process.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

void hb_grid_request_newproject( struct evhttp_request * _request, void * _ud )
{
    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_newproject_in_t in_data;
    in_data.magic_number = hb_node_newproject_magic_number;
    in_data.version_number = hb_node_newproject_version_number;

    strcpy( in_data.db_uri, handle->db_uri );

    hb_sharedmemory_write( &handle->sharedmemory, &in_data, sizeof( in_data ) );

    uint32_t multipart_params_count;
    multipart_params_handle_t multipart_params[8];
    int multipart_parse_error = hb_grid_get_request_params( _request, multipart_params, 8, &multipart_params_count );
    HB_UNUSED( multipart_parse_error );

    char process_command[64];
    sprintf( process_command, "--sm %s"
        , handle->sharedmemory.name
    );

    hb_result_t process_result = hb_process_run( "hb_node_newproject.exe", process_command );
    HB_UNUSED( process_result );

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_newproject_out_t out_data;
    hb_sharedmemory_read( &handle->sharedmemory, &out_data, sizeof( out_data ), HB_NULLPTR );

    if( out_data.magic_number != hb_node_newproject_magic_number )
    {
        return;
    }

    if( out_data.version_number != hb_node_newproject_version_number )
    {
        return;
    }

    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );

    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    char pid16[4] = {0};
    hb_base16_encode( &out_data.pid, 2, pid16, 4, HB_NULLPTR );

    char request_data[HB_GRID_REQUEST_MAX_SIZE];
    size_t request_data_size = sprintf( request_data, "{\"pid\"=\"%.4s\"}"
        , pid16
    );

    evbuffer_add( output_buffer, request_data, request_data_size );

    evhttp_send_reply( _request, HTTP_OK, "", output_buffer );
}