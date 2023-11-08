#include "hb_grid.h"

#include "hb_grid_process_getleaderboard.h"

#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

hb_http_code_t hb_grid_request_getleaderboard( hb_grid_request_handle_t * _args )
{
    const char * arg_user_token;
    if( hb_grid_get_arg_string( _args, "user_token", &arg_user_token ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    uint32_t arg_leaderboard_begin;
    if( hb_grid_get_arg_uint32( _args, "leaderboard_begin", &arg_leaderboard_begin ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    uint32_t arg_leaderboard_end;
    if( hb_grid_get_arg_uint32( _args, "leaderboard_end", &arg_leaderboard_end ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_user_token_t user_token;
    if( hb_grid_get_user_token( _args, arg_user_token, &user_token ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_getleaderboard_in_data_t in_data;
    in_data.project_uid = user_token.project_uid;
    in_data.user_uid = user_token.user_uid;
    in_data.leaderboard_begin = arg_leaderboard_begin;
    in_data.leaderboard_end = arg_leaderboard_end;

    hb_grid_process_lock( _args->process, user_token.user_uid );

    hb_grid_process_getleaderboard_out_data_t out_data;
    hb_result_t result = hb_grid_process_getleaderboard( _args->process, &in_data, &out_data );

    hb_grid_process_unlock( _args->process, user_token.user_uid );

    if( result == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_size_t response_data_size = 0;
    response_data_size += sprintf( _args->response + response_data_size, "{\"code\":0,\"leaderboard\":[" );

    for( uint32_t index = 0; index != out_data.descs_count; ++index )
    {
        if( index != 0 )
        {
            response_data_size += sprintf( _args->response + response_data_size, "," );
        }

        response_data_size += sprintf( _args->response + response_data_size, "{\"uid\":%u,\"nickname\":\"%s\",\"score\":%" PRIu64 "}"
            , out_data.descs[index].user_uid
            , out_data.descs[index].nickname
            , out_data.descs[index].score
        );
    }

    response_data_size += sprintf( _args->response + response_data_size, "]}" );

    return HTTP_OK;
}