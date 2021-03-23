#include "hb_grid.h"

#include "hb_grid_process_getmessageschannel.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_getmessageschannel( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, hb_size_t * _size, const hb_grid_process_cmd_args_t * _args )
{
    HB_UNUSED( _request );

    const char * arg_user_token = _args->arg1;

    hb_user_token_t user_token;
    if( hb_cache_get_token( _process->cache, arg_user_token, 1800, &user_token, sizeof( user_token ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_bool_t required_successful = HB_TRUE;

    hb_grid_process_getmessageschannel_in_data_t in_data;

    in_data.puid = user_token.puid;
    in_data.uuid = user_token.uuid;

    {
        hb_json_handle_t * json_handle;
        if( hb_http_get_request_json( _request, &json_handle ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_get_field_uint32_required( json_handle, "uid", &in_data.cuid, &required_successful ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_get_field_uint32_required( json_handle, "postid", &in_data.postid, &required_successful ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        hb_json_destroy( json_handle );
    }

    if( required_successful == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_lock( _process, user_token.uuid );

    hb_grid_process_getmessageschannel_out_data_t out_data;
    hb_result_t result = hb_grid_process_getmessageschannel( _process, &in_data, &out_data );

    hb_grid_process_unlock( _process, user_token.uuid );

    if( result == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( out_data.code == HB_ERROR_OK )
    {
        hb_size_t response_data_size = sprintf( _response, "{\"code\":0,\"posts\":[" );

        for( uint32_t index = 0; index != out_data.posts_count; ++index )
        {
            if( index != 0 )
            {
                response_data_size += sprintf( _response + response_data_size, "," );
            }

            response_data_size += sprintf( _response + response_data_size, "{\"postid\":%u,\"uuid\":%u,\"message\":\"%s\",\"metainfo\":\"%s\"}"
                , out_data.posts[index].postid
                , out_data.posts[index].uuid
                , out_data.posts[index].message
                , out_data.posts[index].metainfo
            );
        }

        response_data_size += sprintf( _response + response_data_size, "]}" );

        *_size = response_data_size;
    }
    else
    {
        hb_size_t response_data_size = sprintf( _response, "{\"code\":%u}"
            , out_data.code
        );

        *_size = response_data_size;
    }

    return HTTP_OK;
}