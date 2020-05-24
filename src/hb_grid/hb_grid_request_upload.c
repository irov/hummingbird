#include "hb_grid.h"
#include "hb_grid_process_upload.h"

#include "hb_http/hb_http.h"
#include "hb_token/hb_token.h"
#include "hb_utils/hb_multipart.h"
#include "hb_utils/hb_strstre.h"
#include "hb_utils/hb_base16.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <string.h>

hb_http_code_t hb_grid_request_upload( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _args )
{
    const char * account_token = _args->arg1;
    const char * puid = _args->arg2;

    hb_grid_process_upload_in_data_t in_data;
    if( hb_token_base16_decode_string( account_token, &in_data.token ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( hb_base16_decode( puid, HB_UNKNOWN_STRING_SIZE, &in_data.puid, sizeof( in_data.puid ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    uint32_t multipart_params_count;
    multipart_params_handle_t multipart_params[8];
    if( hb_http_get_request_params( _request, multipart_params, 8, &multipart_params_count ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

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

    hb_grid_process_upload_out_data_t out_data;
    if( hb_grid_process_upload( _process, &in_data, &out_data ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    size_t response_data_size = sprintf( _response, "{\"code\":0,\"revision\":%" SCNu64 "}", out_data.revision );

    *_size = response_data_size;

    return HTTP_OK;
}