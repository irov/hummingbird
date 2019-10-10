#include "hb_grid.h"

#include "hb_node_upload/hb_node_upload.h"
#include "hb_process/hb_process.h"
#include "hb_utils/hb_multipart.h"
#include "hb_utils/hb_strstre.h"
#include "hb_utils/hb_base64.h"

void hb_grid_request_upload( struct evhttp_request * _request, void * _ud )
{
    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_upload_in_t in;
    strcpy( in.db_uri, handle->db_uri );

    enum evhttp_cmd_type command_type = evhttp_request_get_command( _request );
    HB_UNUSED( command_type );

    struct evkeyvalq * headers = evhttp_request_get_input_headers( _request );
    HB_UNUSED( headers );

    const char * content_type = evhttp_find_header( headers, "Content-Type" );
    HB_UNUSED( content_type );

    const char * content_type_boundary = hb_strstre( content_type, "boundary=" );
    size_t content_type_boundary_size = strlen( content_type_boundary );

    struct evbuffer * input_buffer = evhttp_request_get_input_buffer( _request );
    
    size_t multipart_length = evbuffer_get_length( input_buffer );

    char multipart[10240];
    ev_ssize_t copyout_buffer_size = evbuffer_copyout( input_buffer, multipart, multipart_length );
    HB_UNUSED( copyout_buffer_size );

    uint32_t multipart_params_count;
    multipart_params_handle_t multipart_params[8];
    int multipart_parse_error = hb_multipart_parse( content_type_boundary, content_type_boundary_size, multipart_params, 8, multipart, multipart_length, &multipart_params_count );
    HB_UNUSED( multipart_parse_error );

    size_t params_puid_size;
    const void * params_puid;
    int multipart_get_puid_error = hb_multipart_get_value( multipart_params, multipart_params_count, "puid", &params_puid, &params_puid_size );
    HB_UNUSED( multipart_get_puid_error );

    hb_base64_decode( params_puid, params_puid_size, in.puid, 12, HB_NULLPTR );

    size_t params_data_size;
    const void * params_data;
    int multipart_get_data_error = hb_multipart_get_value( multipart_params, multipart_params_count, "data", &params_data, &params_data_size );
    HB_UNUSED( multipart_get_data_error );

    memcpy( in.data, params_data, params_data_size );
    in.data_size = params_data_size;

    hb_sharedmemory_write( &handle->sharedmemory, &in, sizeof( hb_node_upload_in_t ) );

    char process_command[64];
    sprintf( process_command, "--sm %s"
        , handle->sharedmemory.name
    );

    hb_process_run( "hb_node_upload.exe", process_command );

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_upload_out_t out;
    hb_sharedmemory_read( &handle->sharedmemory, &out, sizeof( hb_node_upload_out_t ), HB_NULLPTR );

    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );

    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    char result[256];
    size_t result_size = sprintf( result, "{}" );

    evbuffer_add( output_buffer, result, result_size );

    evhttp_send_reply( _request, HTTP_OK, "", output_buffer );
}