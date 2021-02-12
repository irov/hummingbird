#include "hb_grid.h"

#include "hb_events/hb_events.h"
#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>
#include <stdio.h>

hb_http_code_t hb_grid_request_geteventstopic( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, hb_size_t * _size, const hb_grid_process_cmd_args_t * _args )
{
    const char * arg_user_token = _args->arg1;

    hb_user_token_t token_handle;
    if( hb_cache_get_token( _process->cache, arg_user_token, 1800, &token_handle, sizeof( token_handle ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_uid_t tuid;
    uint32_t index;

    hb_bool_t required_successful = HB_TRUE;

    {
        hb_json_handle_t * json_handle;
        if( hb_http_get_request_json( _request, &json_handle ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_get_field_uint32_required( json_handle, "uid", &tuid, &required_successful ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_get_field_uint32_required( json_handle, "index", &index, &required_successful ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        hb_json_destroy( json_handle );
    }

    if( required_successful == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_mutex_handle_t * mutex_handle = _process->mutex_handles + token_handle.uuid % _process->mutex_count;
    hb_mutex_lock( mutex_handle->mutex );

    hb_events_topic_t topic;
    hb_error_code_t code;
    hb_result_t result = hb_events_get_topic( _process->events, _process->cache, _process->db_client, token_handle.puid, tuid, &topic, &code );

    hb_mutex_unlock( mutex_handle->mutex );

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

    if( index != ~0U && topic.index == index )
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