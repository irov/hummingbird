#include "hb_grid.h"

#include "hb_grid_process_newmessageschannel.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_newmessageschannel( hb_grid_request_handle_t * _args )
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

    uint32_t arg_messageschannel_maxpost;
    if( hb_grid_get_arg_uint32( _args, "messageschannel_maxpost", &arg_messageschannel_maxpost ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_account_token_t account_token;
    if( hb_grid_get_account_token( _args, arg_account_token, &account_token ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_grid_process_newmessageschannel_in_data_t in_data;
    in_data.auid = account_token.account_uid;
    
    if( hb_grid_get_uid( arg_project_uid, &in_data.project_uid ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    in_data.messageschannel_maxpost = arg_messageschannel_maxpost;

    hb_grid_process_lock( _args->process, account_token.account_uid );

    hb_grid_process_newmessageschannel_out_data_t out_data;
    hb_result_t result = hb_grid_process_newmessageschannel( _args->process, &in_data, &out_data );

    hb_grid_process_unlock( _args->process, account_token.account_uid );

    if( result == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":0,\"uid\":%u}"
        , out_data.cuid
    );
    
    return HTTP_OK;
}