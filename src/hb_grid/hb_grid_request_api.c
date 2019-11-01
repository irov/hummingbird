#include "hb_grid_request.h"

#include "hb_node_api/hb_node_api.h"

#include "hb_token/hb_token.h"
#include "hb_process/hb_process.h"
#include "hb_utils/hb_base64.h"

void hb_grid_request_api( struct evhttp_request * _request, void * _ud )
{    
    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );

    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

    hb_node_api_in_t in_data;

    strcpy( in_data.cache_uri, handle->cache_uri );
    in_data.cache_port = handle->cache_port;

    strcpy( in_data.db_uri, handle->db_uri );

    const char * token;
    if( hb_grid_get_request_header( _request, "X-Token", &token ) == 0 )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    hb_token_base16_decode( token, in_data.token );

    const char * method;
    if( hb_grid_get_request_header( _request, "X-Method", &method ) == 0 )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    in_data.category = e_hb_node_api;
    strcpy( in_data.method, method );

    if( hb_grid_get_request_data( _request, in_data.data, HB_DATA_MAX_SIZE, &in_data.data_size ) == 0 )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    hb_node_write_in_data( &handle->sharedmemory, &in_data, sizeof( in_data ), hb_node_api_magic_number, hb_node_api_version_number );
    
    hb_bool_t process_successful;
    if( hb_process_run( "hb_node_api.exe", handle->sharedmemory.name, &process_successful ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    if( process_successful == HB_FALSE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    hb_node_api_out_t out_data;
    uint32_t out_code;
    if( hb_node_read_out_data( &handle->sharedmemory, &out_data, sizeof( out_data ), hb_node_api_magic_number, hb_node_api_version_number, &out_code ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    if( out_code != 0 )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        return;
    }

    char response_data[HB_GRID_REQUEST_DATA_MAX_SIZE];
    size_t response_data_size = sprintf( response_data, "{\"code\": 0, \"successful\": %u, \"data\": %.*s}"
        , out_data.successful
        , out_data.response_size
        , out_data.response_data
    );

    evbuffer_add( output_buffer, response_data, response_data_size );

    evhttp_send_reply( _request, HTTP_OK, "", output_buffer );
}