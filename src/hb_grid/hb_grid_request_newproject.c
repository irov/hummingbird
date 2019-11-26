#include "hb_grid_request.h"

#include "hb_node_newproject/hb_node_newproject.h"

#include "hb_process/hb_process.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

int hb_grid_request_newproject( struct evhttp_request * _request, struct hb_grid_process_handle_t * _handle, char * _response, size_t * _size )
{
    HB_UNUSED( _request );

    hb_node_newproject_in_t in_data;

    if( hb_node_write_in_data( _handle->sharedmemory, &in_data, sizeof( in_data ), &_handle->config ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_bool_t process_successful;
    if( hb_process_run( "hb_node_newproject.exe", _handle->sharedmemory, &process_successful ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( process_successful == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_node_newproject_out_t out_data;
    hb_node_code_t out_code;
    if( hb_node_read_out_data( _handle->sharedmemory, &out_data, sizeof( out_data ), &out_code ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( out_code != e_node_ok )
    {
        return HTTP_BADREQUEST;
    }

    hb_pid16_t pid16;
    hb_base16_encode( &out_data.pid, sizeof( out_data.pid ), pid16, sizeof( pid16 ), HB_NULLPTR );

    size_t response_data_size = sprintf( _response, "{\"code\": 0, \"pid\": \"%.*s\"}"
        , sizeof( pid16 )
        , pid16
    );

    *_size = response_data_size;

    return HTTP_OK;
}