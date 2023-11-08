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
    const char * arg_account_token;
    if( hb_grid_get_arg_string( _args, "account_token", &arg_account_token ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    const char * arg_project_uid;
    if( hb_grid_get_arg_string( _args, "project_uid", &arg_project_uid ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    const char * arg_eventstopic_name;
    if( hb_grid_get_arg_string( _args, "eventstopic_name", &arg_eventstopic_name ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    uint32_t arg_eventstopic_delay;
    if( hb_grid_get_arg_uint32( _args, "eventstopic_delay", &arg_eventstopic_delay ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_account_token_t account_token;
    if( hb_grid_get_account_token( _args, arg_account_token, &account_token ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_uid_t project_uid;
    if( hb_grid_get_uid( arg_project_uid, &project_uid ) == HB_FAILURE )
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

    snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":0,\"uid\":%u}"
        , topic_uid
    );
    
    return HTTP_OK;
}