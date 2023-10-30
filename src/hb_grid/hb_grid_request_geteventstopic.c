#include "hb_grid.h"

#include "hb_events/hb_events.h"
#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>
#include <stdio.h>

hb_http_code_t hb_grid_request_geteventstopic( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size )
{
    hb_bool_t required = HB_TRUE;

    const char * arg_user_token;
    hb_json_get_field_string_required( _data, "user_token", &arg_user_token, HB_NULLPTR, &required );

    uint32_t arg_eventstopic_uid;
    hb_json_get_field_uint32_required( _data, "eventstopic_uid", &arg_eventstopic_uid, &required );

    uint32_t arg_eventstopic_index;
    hb_json_get_field_uint32_required( _data, "eventstopic_index", &arg_eventstopic_index, &required );

    if( required == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_user_token_t user_token;
    if( hb_cache_get_token( _process->cache, arg_user_token, 1800, &user_token, sizeof( user_token ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_lock( _process, user_token.user_uid );

    hb_events_topic_t topic;
    hb_error_code_t code;
    hb_result_t result = hb_events_get_topic( _process->events, _process->cache, _process->db_client, user_token.project_uid, arg_eventstopic_uid, &topic, &code );

    hb_grid_process_unlock( _process, user_token.user_uid );

    if( result == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( code != HB_ERROR_OK )
    {
        hb_size_t response_data_size = sprintf( _response, "{\"code\":%u}"
            , code
        );

        *_size = response_data_size;

        return HTTP_OK;
    }

    if( arg_eventstopic_index != ~0U && topic.index == arg_eventstopic_index )
    {
        hb_size_t response_data_size = sprintf( _response, "{\"code\":0,\"index\":%u}"
            , topic.index
        );

        *_size = response_data_size;

        return HTTP_OK;
    }

    hb_size_t response_data_size = sprintf( _response, "{\"code\":0,\"index\":%u,\"message\":%s}"
        , topic.index
        , topic.message
    );

    *_size = response_data_size;

    return HTTP_OK;
}