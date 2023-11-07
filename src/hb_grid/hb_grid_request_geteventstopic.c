#include "hb_grid.h"

#include "hb_events/hb_events.h"
#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>
#include <stdio.h>

hb_http_code_t hb_grid_request_geteventstopic( hb_grid_request_handle_t * _args )
{
    const char * arg_user_token;
    if( hb_json_get_field_string( _args->data, "user_token", &arg_user_token, HB_NULLPTR ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get user token" );

        return HTTP_BADREQUEST;
    }

    uint32_t arg_eventstopic_uid;
    if( hb_json_get_field_uint32( _args->data, "eventstopic_uid", &arg_eventstopic_uid ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get eventstopic uid" );

        return HTTP_BADREQUEST;
    }

    uint32_t arg_eventstopic_index;
    if( hb_json_get_field_uint32( _args->data, "eventstopic_index", &arg_eventstopic_index ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get eventstopic index" );

        return HTTP_BADREQUEST;
    }

    hb_user_token_t user_token;
    if( hb_cache_get_token( _args->process->cache, arg_user_token, 1800, &user_token, sizeof( user_token ), HB_NULLPTR ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get user token" );

        return HTTP_BADREQUEST;
    }

    hb_grid_process_lock( _args->process, user_token.user_uid );

    hb_events_topic_t topic;
    hb_error_code_t code;
    hb_result_t result = hb_events_get_topic( _args->process->events, _args->process->cache, _args->process->db_client, user_token.project_uid, arg_eventstopic_uid, &topic, &code );

    hb_grid_process_unlock( _args->process, user_token.user_uid );

    if( result == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( code != HB_ERROR_OK )
    {
        snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":%u}"
            , code
        );

        return HTTP_OK;
    }

    if( arg_eventstopic_index != ~0U && topic.index == arg_eventstopic_index )
    {
        snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":0,\"index\":%u}"
            , topic.index
        );

        return HTTP_OK;
    }

    snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":0,\"index\":%u,\"message\":%s}"
        , topic.index
        , topic.message
    );

    return HTTP_OK;
}