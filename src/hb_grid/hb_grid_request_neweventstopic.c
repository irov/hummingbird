#include "hb_grid.h"

#include "hb_events/hb_events.h"
#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_neweventstopic( hb_grid_request_handle_t * _args )
{
    hb_bool_t required = HB_TRUE;

    const char * arg_account_token;
    hb_json_get_field_string_required( _args->data, "account_token", &arg_account_token, HB_NULLPTR, &required );

    const char * arg_project_uid;
    hb_json_get_field_string_required( _args->data, "project_uid", &arg_project_uid, HB_NULLPTR, &required );

    const char * arg_eventstopic_name;
    hb_json_get_field_string_required( _args->data, "eventstopic_name", &arg_eventstopic_name, HB_NULLPTR, &required );

    uint32_t arg_eventstopic_delay;
    hb_json_get_field_uint32_required( _args->data, "eventstopic_delay", &arg_eventstopic_delay, &required );

    if( required == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_account_token_t account_token;
    if( hb_cache_get_token( _args->process->cache, arg_account_token, 1800, &account_token, sizeof( hb_user_token_t ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_uid_t project_uid;
    if( hb_base16_decode( arg_project_uid, HB_UNKNOWN_STRING_SIZE, &project_uid, sizeof( hb_uid_t ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_lock( _args->process, account_token.account_uid );

    hb_uid_t topic_uid;
    hb_result_t result = hb_events_new_topic( _args->process->events, _args->process->db_client, project_uid, arg_eventstopic_name, arg_eventstopic_delay, &topic_uid );

    hb_grid_process_unlock( _args->process, account_token.account_uid );

    if( result == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":0,\"topic_uid\":%u}"
        , topic_uid
    );
    
    return HTTP_OK;
}