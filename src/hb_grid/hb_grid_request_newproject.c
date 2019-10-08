#include "hb_grid.h"

#include "hb_node_newproject/hb_node_newproject.h"
#include "hb_process/hb_process.h"
#include "hb_utils/hb_base64.h"

void hb_grid_request_newproject( struct evhttp_request * _request, void * _ud )
{
    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_newproject_in_t in;

    const char * mongodb_uri = "mongodb://localhost:27017";
    strcpy( in.db_uri, mongodb_uri );

    hb_sharedmemory_write( &handle->sharedmemory, &in, sizeof( hb_node_newproject_in_t ) );    

    enum evhttp_cmd_type command_type = evhttp_request_get_command( _request );
    HB_UNUSED( command_type );

    struct evkeyvalq * headers = evhttp_request_get_input_headers( _request );
    HB_UNUSED( headers );

    const char * content_type = evhttp_find_header( headers, "Content-Type" );
    HB_UNUSED( content_type );

    struct evbuffer * input_buffer = evhttp_request_get_input_buffer( _request );
    HB_UNUSED( input_buffer );

    size_t length = evbuffer_get_length( input_buffer );

    uint8_t copyout_buffer[2048];
    ev_ssize_t copyout_buffer_size = evbuffer_copyout( input_buffer, copyout_buffer, length );

    hb_sharedmemory_write( &handle->sharedmemory, copyout_buffer, copyout_buffer_size );

    char process_command[64];
    sprintf( process_command, "--sm %s"
        , handle->sharedmemory.name
    );

    int process_result = hb_process_run( "hb_node_newproject.exe", process_command );
    HB_UNUSED( process_result );

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_newproject_out_t out;
    hb_sharedmemory_read( &handle->sharedmemory, &out, sizeof( hb_node_newproject_out_t ), HB_NULLPTR );

    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );

    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    char puid64[25];
    hb_base64_decode( out.puid, 12, puid64, 25, HB_NULLPTR );

    char request[256];
    size_t request_size = sprintf( request, "{\"puid\"=\"%s\"}"
        , puid64
    );

    evbuffer_add( output_buffer, request, request_size );

    evhttp_send_reply( _request, HTTP_OK, "", output_buffer );
}