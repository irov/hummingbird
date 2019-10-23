#include "hb_grid_request.h"

#include "hb_node_upload/hb_node_upload.h"
#include "hb_process/hb_process.h"
#include "hb_utils/hb_multipart.h"
#include "hb_utils/hb_strstre.h"
#include "hb_utils/hb_base64.h"

void hb_grid_request_upload( struct evhttp_request * _request, void * _ud )
{
    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_upload_in_t in_data;
    strcpy( in_data.db_uri, handle->db_uri );

    uint32_t multipart_params_count;
    multipart_params_handle_t multipart_params[8];
    int multipart_parse_error = hb_grid_get_request_params( _request, multipart_params, 8, &multipart_params_count );
    HB_UNUSED( multipart_parse_error );

    size_t params_puid_size;
    const void * params_puid;
    int multipart_get_puid_error = hb_multipart_get_value( multipart_params, multipart_params_count, "puid", &params_puid, &params_puid_size );
    HB_UNUSED( multipart_get_puid_error );

    hb_base64_decode( params_puid, params_puid_size, in_data.puid, 12, HB_NULLPTR );

    size_t params_data_size;
    const void * params_data;
    int multipart_get_data_error = hb_multipart_get_value( multipart_params, multipart_params_count, "data", &params_data, &params_data_size );
    HB_UNUSED( multipart_get_data_error );

    memcpy( in_data.data, params_data, params_data_size );
    in_data.data_size = params_data_size;

    if( hb_sharedmemory_write( &handle->sharedmemory, &in_data, sizeof( in_data ) ) == 0 )
    {
        evhttp_send_reply( _request, HTTP_INTERNAL, "", HB_NULLPTR );

        return;
    }

    char process_command[64];
    sprintf( process_command, "--sm %s"
        , handle->sharedmemory.name
    );

    hb_result_t result_process = hb_process_run( "hb_node_upload.exe", process_command );
    HB_UNUSED( result_process );

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_upload_out_t out_data;
    hb_sharedmemory_read( &handle->sharedmemory, &out_data, sizeof( out_data ), HB_NULLPTR );

    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );

    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    char request_data[HB_GRID_REQUEST_MAX_SIZE];
    size_t request_data_size = sprintf( request_data, "{}" );

    evbuffer_add( output_buffer, request_data, request_data_size );

    evhttp_send_reply( _request, HTTP_OK, "", output_buffer );
}