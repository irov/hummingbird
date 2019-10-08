#include "hb_grid.h"

#include "hb_process/hb_process.h"

void hb_grid_request_api( struct evhttp_request * _request, void * _ud )
{    
    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

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

    hb_process_run( "hb_node_api.exe", process_command );

    hb_sharedmemory_rewind( &handle->sharedmemory );

    size_t process_result_size;
    char process_result[2048];
    hb_sharedmemory_read( &handle->sharedmemory, process_result, 2048, &process_result_size );

    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );
    
    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    evbuffer_add( output_buffer, process_result, process_result_size );

    evhttp_send_reply( _request, HTTP_OK, "", output_buffer );
}