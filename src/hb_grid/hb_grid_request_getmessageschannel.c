#include "hb_grid.h"

#include "hb_grid_process_getmessageschannel.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_getmessageschannel( hb_grid_request_handle_t * _args )
{
    const char * arg_user_token;
    if( hb_grid_get_arg_string( _args, "user_token", &arg_user_token ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    uint32_t arg_messageschannel_uid;
    if( hb_grid_get_arg_uint32( _args, "messageschannel_uid", &arg_messageschannel_uid ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    uint32_t arg_messageschannel_postid;
    if( hb_grid_get_arg_uint32( _args, "messageschannel_postid", &arg_messageschannel_postid ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_user_token_t user_token;
    if( hb_grid_get_user_token( _args, arg_user_token, &user_token ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_grid_process_getmessageschannel_in_data_t in_data;
    in_data.project_uid = user_token.project_uid;
    in_data.user_uid = user_token.user_uid;
    in_data.messageschannel_uid = arg_messageschannel_uid;
    in_data.messageschannel_postid = arg_messageschannel_postid;

    hb_grid_process_lock( _args->process, user_token.user_uid );

    hb_grid_process_getmessageschannel_out_data_t out_data;
    hb_result_t result = hb_grid_process_getmessageschannel( _args->process, &in_data, &out_data );

    hb_grid_process_unlock( _args->process, user_token.user_uid );

    if( result == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( out_data.code == HB_ERROR_OK )
    {
        hb_size_t response_data_offset = snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":0,\"posts\":[" );

        for( uint32_t index = 0; index != out_data.posts_count; ++index )
        {
            if( index != 0 )
            {
                response_data_offset += sprintf( _args->response + response_data_offset, "," );
            }

            response_data_offset += snprintf( _args->response + response_data_offset, HB_GRID_RESPONSE_DATA_MAX_SIZE - response_data_offset, "{\"postid\":%u,\"uuid\":%u,\"message\":\"%s\",\"metainfo\":\"%s\"}"
                , out_data.posts[index].postid
                , out_data.posts[index].user_uid
                , out_data.posts[index].message
                , out_data.posts[index].metainfo
            );
        }

        snprintf( _args->response + response_data_offset, HB_GRID_RESPONSE_DATA_MAX_SIZE - response_data_offset, "]}" );
    }
    else
    {
        snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":%u}"
            , out_data.code
        );
    }

    return HTTP_OK;
}