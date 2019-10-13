#include "hb_grid_request.h"

#include "hb_node_universe/hb_node_universe.h"

#include "hb_process/hb_process.h"
#include "hb_utils/hb_multipart.h"
#include "hb_utils/hb_strstre.h"
#include "hb_utils/hb_base64.h"

void hb_grid_request_universe( struct evhttp_request * _request, void * _ud )
{
    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_universe_in_t in_data;
    strcpy( in_data.db_uri, handle->db_uri );
   
    in_data.token_expire_time = 86400;

    if( hb_sharedmemory_write( &handle->sharedmemory, &in_data, sizeof( in_data ) ) == 0 )
    {
        evhttp_send_reply( _request, HTTP_INTERNAL, "", HB_NULLPTR );

        return;
    }

    char process_command[64];
    sprintf( process_command, "--sm %s"
        , handle->sharedmemory.name
    );

    hb_process_run( "hb_node_universe.exe", process_command );

    hb_sharedmemory_rewind( &handle->sharedmemory );

    hb_node_universe_out_t out_data;
    hb_sharedmemory_read( &handle->sharedmemory, &out_data, sizeof( out_data ), HB_NULLPTR );

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