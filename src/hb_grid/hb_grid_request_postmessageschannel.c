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
    hb_bool_t required = HB_TRUE;

    const char * arg_user_token;
    hb_json_get_field_string_required( _args->data, "user_token", &arg_user_token, HB_NULLPTR, &required );

    uint32_t arg_messageschannel_uid;
    hb_json_get_field_uint32_required( _args->data, "messageschannel_uid", &arg_messageschannel_uid, &required );

    const char * arg_messageschannel_message;
    hb_json_get_field_string_required( _args->data, "messageschannel_message", &arg_messageschannel_message, HB_NULLPTR, &required );

    const char * arg_messageschannel_metainfo;
    hb_json_get_field_string_required( _args->data, "messageschannel_metainfo", &arg_messageschannel_metainfo, HB_NULLPTR, &required );

    if( required == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_user_token_t user_token;
    if( hb_cache_get_token( _args->process->cache, arg_user_token, 1800, &user_token, sizeof( user_token ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
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