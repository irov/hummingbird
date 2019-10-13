#include "hb_grid_request.h"

#include "hb_node_api/hb_node_api.h"

#include "hb_process/hb_process.h"
#include "hb_utils/hb_base64.h"

void hb_grid_request_api( struct evhttp_request * _request, void * _ud )
{    
    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_api_in_t in_data;

    strcpy( in_data.db_uri, handle->db_uri );

    const char * token;
    if( hb_grid_get_request_header( _request, "X-Token", &token ) == 0 )
    {
        return;
    }

    hb_base64_decode( token, strlen( token ), in_data.token, 12, HB_NULLPTR );

    const char * method;
    if( hb_grid_get_request_header( _request, "X-Method", &method ) == 0 )
    {
        return;
    }

    strcpy( in_data.method, method );

    if( hb_grid_get_request_data( _request, in_data.data, 10240, &in_data.data_size ) == 0 )
    {
        return;
    }

    hb_sharedmemory_write( &handle->sharedmemory, &in_data, sizeof( in_data ) );
    
    char process_command[64];
    sprintf( process_command, "--sm %s"
        , handle->sharedmemory.name
    );

    hb_process_run( "hb_node_api.exe", process_command );

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_api_out_t out_data;
    hb_sharedmemory_read( &handle->sharedmemory, &out_data, sizeof( out_data ), HB_NULLPTR );

    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );
    
    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    evbuffer_add( output_buffer, out_data.result, out_data.result_size );

    evhttp_send_reply( _request, HTTP_OK, "", output_buffer );
}