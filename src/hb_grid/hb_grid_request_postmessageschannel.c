#include "hb_grid.h"

#include "hb_grid_process_postmessageschannel.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_postmessageschannel( hb_grid_request_handle_t * _args )
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

    const char * arg_messageschannel_message;
    if( hb_grid_get_arg_string( _args, "messageschannel_message", &arg_messageschannel_message ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    const char * arg_messageschannel_metainfo;
    if( hb_grid_get_arg_string( _args, "messageschannel_metainfo", &arg_messageschannel_metainfo ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_user_token_t user_token;
    if( hb_grid_get_user_token( _args, arg_user_token, &user_token ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_postmessageschannel_in_data_t in_data;
    in_data.user_uid = user_token.user_uid;
    in_data.project_uid = user_token.project_uid;
    in_data.messageschannel_uid = arg_messageschannel_uid;
    strncpy( in_data.messageschannel_message, arg_messageschannel_message, 256 );
    strncpy( in_data.messageschannel_metainfo, arg_messageschannel_metainfo, 256 );

    hb_grid_process_lock( _args->process, user_token.user_uid );

    hb_grid_process_postmessageschannel_out_data_t out_data;
    hb_result_t result = hb_grid_process_postmessageschannel( _args->process, &in_data, &out_data );

    hb_grid_process_unlock( _args->process, user_token.user_uid );

    if( result == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( out_data.code == HB_ERROR_OK )
    {
        snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":0,\"postid\":%u}"
            , out_data.postid
        );
    }
    else
    {
        snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":%u}"
            , out_data.code
        );
    }

    return HTTP_OK;
}