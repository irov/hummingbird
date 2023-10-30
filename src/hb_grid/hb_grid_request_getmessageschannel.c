#include "hb_grid.h"

#include "hb_grid_process_getmessageschannel.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_getmessageschannel( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size )
{
    hb_bool_t required = HB_TRUE;

    const char * arg_user_token;
    hb_json_get_field_string_required( _data, "user_token", &arg_user_token, HB_NULLPTR, &required );

    uint32_t arg_messageschannel_uid;
    hb_json_get_field_uint32_required( _data, "messageschannel_uid", &arg_messageschannel_uid, &required );

    uint32_t arg_messageschannel_postid;
    hb_json_get_field_uint32_required( _data, "messageschannel_postid", &arg_messageschannel_postid, &required );

    if( required == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_user_token_t user_token;
    if( hb_cache_get_token( _process->cache, arg_user_token, 1800, &user_token, sizeof( user_token ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_grid_process_getmessageschannel_in_data_t in_data;
    in_data.project_uid = user_token.project_uid;
    in_data.user_uid = user_token.user_uid;
    in_data.messageschannel_uid = arg_messageschannel_uid;
    in_data.messageschannel_postid = arg_messageschannel_postid;

    hb_grid_process_lock( _process, user_token.user_uid );

    hb_grid_process_getmessageschannel_out_data_t out_data;
    hb_result_t result = hb_grid_process_getmessageschannel( _process, &in_data, &out_data );

    hb_grid_process_unlock( _process, user_token.user_uid );

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
                , out_data.posts[index].user_uid
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