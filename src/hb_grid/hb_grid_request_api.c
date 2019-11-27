#include "hb_grid_request.h"

#include "hb_node_api/hb_node_api.h"

#include "hb_token/hb_token.h"
#include "hb_process/hb_process.h"
#include "hb_utils/hb_base64.h"

int hb_grid_request_api( struct evhttp_request * _request, struct hb_grid_process_handle_t * _handle, char * _response, size_t * _size )
{    
    hb_node_api_in_t in_data;

    const char * token;
    if( hb_grid_get_request_header( _request, "X-Token", &token ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_token_base16_decode( token, in_data.token );

    const char * method;
    if( hb_grid_get_request_header( _request, "X-Method", &method ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    in_data.category = e_hb_node_api;
    strcpy( in_data.method, method );

    if( hb_grid_get_request_data( _request, in_data.data, HB_DATA_MAX_SIZE, &in_data.data_size ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( hb_node_write_in_data( _handle->sharedmemory, &in_data, sizeof( in_data ), _handle->config ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_bool_t process_successful;
    if( hb_process_run( "hb_node_api.exe", _handle->sharedmemory, &process_successful ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( process_successful == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_node_api_out_t out_data;
    hb_node_code_t out_code;
    if( hb_node_read_out_data( _handle->sharedmemory, &out_data, sizeof( out_data ), &out_code ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( out_code != 0 )
    {
        return HTTP_BADREQUEST;
    }

    if( out_data.successful == HB_TRUE )
    {
        size_t response_data_size = sprintf( _response, "{\"code\": 0, \"successful\": true, \"data\": %.*s, \"stat\": {\"memory_used\":%u, \"call_used\":%u}}"
            , out_data.response_size
            , out_data.response_data
            , out_data.memory_used
            , out_data.call_used
        );

        *_size = response_data_size;
    }
    else
    {
        size_t response_data_size = sprintf( _response, "{\"code\": 0, \"successful\": false, \"method_found\": %s}"
            , out_data.method_found == HB_TRUE ? "true" : "false"
        );

        *_size = response_data_size;
    }

    return HTTP_OK;
}