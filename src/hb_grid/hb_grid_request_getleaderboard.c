#include "hb_grid.h"

#include "hb_grid_process_getleaderboard.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_getleaderboard( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, hb_size_t * _size, const hb_grid_process_cmd_args_t * _args )
{
    const char * user_token = _args->arg1;

    hb_bool_t required_successful = HB_TRUE;

    hb_grid_process_getleaderboard_in_data_t in_data;

    if( hb_token_base16_decode_string( user_token, &in_data.token ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    {
        hb_json_handle_t * json_handle;
        if( hb_http_get_request_json( _request, &json_handle ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_get_field_uint32_required( json_handle, "begin", &in_data.begin, &required_successful ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_get_field_uint32_required( json_handle, "end", &in_data.end, &required_successful ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        hb_json_destroy( json_handle );
    }

    if( required_successful == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_getleaderboard_out_data_t out_data;
    if( hb_grid_process_getleaderboard( _process, &in_data, &out_data ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_size_t response_data_size = 0;
    response_data_size += sprintf( _response + response_data_size, "{\"code\":0,\"leaderboard\":[" );

    for( uint32_t index = 0; index != out_data.descs_count; ++index )
    {
        if( index != 0 )
        {
            response_data_size += sprintf( _response + response_data_size, "," );
        }

        response_data_size += sprintf( _response + response_data_size, "{\"uid\":%u,\"nickname\":\"%s\",\"score\":%u}"
            , out_data.descs[index].uuid
            , out_data.descs[index].nickname
            , out_data.descs[index].score
        );
    }

    response_data_size += sprintf( _response + response_data_size, "]}" );

    *_size = response_data_size;

    return HTTP_OK;
}