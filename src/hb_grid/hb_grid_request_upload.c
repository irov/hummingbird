#include "hb_grid_request.h"

#include "hb_node_upload/hb_node_upload.h"
#include "hb_process/hb_process.h"
#include "hb_utils/hb_multipart.h"
#include "hb_utils/hb_strstre.h"
#include "hb_utils/hb_base16.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

int hb_grid_request_upload( struct evhttp_request * _request, struct hb_grid_process_handle_t * _handle, char * _response, size_t * _size )
{
    hb_node_upload_in_t in_data;

    uint32_t multipart_params_count;
    multipart_params_handle_t multipart_params[8];
    if( hb_grid_get_request_params( _request, multipart_params, 8, &multipart_params_count ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    size_t params_pid_size;
    const void * params_pid;
    if( hb_multipart_get_value( multipart_params, multipart_params_count, "pid", &params_pid, &params_pid_size ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_base16_decode( params_pid, params_pid_size, &in_data.pid, sizeof( in_data.pid ), HB_NULLPTR );

    size_t params_data_size;
    const void * params_data;
    if( hb_multipart_get_value( multipart_params, multipart_params_count, "data", &params_data, &params_data_size ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( params_data_size > HB_DATA_MAX_SIZE )
    {
        return HTTP_BADREQUEST;
    }

    memcpy( in_data.script_source, params_data, params_data_size );
    in_data.script_source_size = params_data_size;

    if( hb_node_write_in_data( &_handle->sharedmemory, &in_data, sizeof( in_data ), &_handle->config ) == 0 )
    {
        return HTTP_BADREQUEST;
    }

    hb_bool_t process_successful;
    if( hb_process_run( "hb_node_upload.exe", _handle->sharedmemory.name, &process_successful ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( process_successful == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_node_upload_out_t out_data;
    hb_node_code_t out_code;
    if( hb_node_read_out_data( &_handle->sharedmemory, &out_data, sizeof( out_data ), &out_code ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( out_code != e_node_ok )
    {
        return HTTP_BADREQUEST;
    }

    size_t response_data_size = sprintf( _response, "{\"code\": 0, \"revision\": %" SCNu64 "}", out_data.revision );

    *_size = response_data_size;

    return HTTP_OK;
}