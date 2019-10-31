#include "hb_grid_request.h"

#include "hb_node_upload/hb_node_upload.h"
#include "hb_process/hb_process.h"
#include "hb_utils/hb_multipart.h"
#include "hb_utils/hb_strstre.h"
#include "hb_utils/hb_base16.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

void hb_grid_request_upload( struct evhttp_request * _request, void * _ud )
{
    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );

    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

    hb_node_upload_in_t in_data;
    strcpy( in_data.db_uri, handle->db_uri );

    uint32_t multipart_params_count;
    multipart_params_handle_t multipart_params[8];
    if( hb_grid_get_request_params( _request, multipart_params, 8, &multipart_params_count ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "hb_grid_get_request_params", output_buffer );

        return;
    }

    size_t params_pid_size;
    const void * params_pid;
    if( hb_multipart_get_value( multipart_params, multipart_params_count, "pid", &params_pid, &params_pid_size ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "hb_multipart_get_value", output_buffer );

        return;
    }

    hb_base16_decode( params_pid, params_pid_size, &in_data.pid, sizeof( in_data.pid ), HB_NULLPTR );

    size_t params_data_size;
    const void * params_data;
    if( hb_multipart_get_value( multipart_params, multipart_params_count, "data", &params_data, &params_data_size ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "hb_multipart_get_value", output_buffer );

        return;
    }

    if( params_data_size > HB_DATA_MAX_SIZE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "params_data_size > HB_DATA_MAX_SIZE", HB_NULLPTR );

        return;
    }

    memcpy( in_data.data, params_data, params_data_size );
    in_data.data_size = params_data_size;

    if( hb_node_write_in_data( &handle->sharedmemory, &in_data, sizeof( in_data ), hb_node_upload_magic_number, hb_node_upload_version_number ) == 0 )
    {
        evhttp_send_reply( _request, HTTP_INTERNAL, "hb_node_write_in_data", HB_NULLPTR );

        return;
    }

    char process_command[64];
    sprintf( process_command, "--sm %s"
        , handle->sharedmemory.name
    );

    hb_bool_t process_successful;
    if( hb_process_run( "hb_node_upload.exe", process_command, &process_successful ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "hb_process_run", output_buffer );

        return;
    }

    if( process_successful == HB_FALSE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "process_successful", output_buffer );

        return;
    }

    hb_node_upload_out_t out_data;
    if( hb_node_read_out_data( &handle->sharedmemory, &out_data, sizeof( out_data ), hb_node_upload_magic_number, hb_node_upload_version_number ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "hb_node_read_out_data", output_buffer );

        return;
    }

    char response_data[HB_GRID_REQUEST_DATA_MAX_SIZE];
    size_t response_data_size = sprintf( response_data, "{\"code\": 0, \"revision\": %" SCNu64 "}", out_data.revision );

    evbuffer_add( output_buffer, response_data, response_data_size );

    evhttp_send_reply( _request, HTTP_OK, "", output_buffer );
}