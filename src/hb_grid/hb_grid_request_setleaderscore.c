#include "hb_grid.h"

#include "hb_grid_process_setleaderscore.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

hb_http_code_t hb_grid_request_setleaderscore( hb_grid_request_handle_t * _args )
{
    const char * arg_user_token;
    if( hb_grid_get_arg_string( _args, "user_token", &arg_user_token ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    uint64_t arg_score;
    if( hb_grid_get_arg_uint64( _args, "score", &arg_score ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_user_token_t user_token;
    if( hb_grid_get_user_token( _args, arg_user_token, &user_token ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_setleaderscore_in_data_t in_data;
    in_data.user_uid = user_token.user_uid;
    in_data.project_uid = user_token.project_uid;
    in_data.score = arg_score;

    hb_grid_process_lock( _args->process, user_token.user_uid );

    hb_grid_process_setleaderscore_out_data_t out_data;
    hb_result_t result = hb_grid_process_setleadscore( _args->process, &in_data, &out_data );

    hb_grid_process_unlock( _args->process, user_token.user_uid );

    if( result == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":0}" );

    return HTTP_OK;
}