#include "hb_grid_process_upload.h"

#include "hb_http/hb_http.h"
#include "hb_token/hb_token.h"
#include "hb_log/hb_log.h"
#include "hb_utils/hb_multipart.h"
#include "hb_utils/hb_strstre.h"
#include "hb_utils/hb_base16.h"
#include "hb_utils/hb_base64.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <string.h>

hb_http_code_t hb_grid_request_upload( hb_grid_request_handle_t * _args )
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

    const char * arg_code;
    hb_size_t arg_code_size;
    if( hb_grid_get_arg_string_size( _args, "code", &arg_code, &arg_code_size ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    hb_account_token_t account_token;
    if( hb_grid_get_account_token( _args, arg_account_token, &account_token ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get account token" );

        return HTTP_BADREQUEST;
    }

    hb_grid_process_upload_in_data_t in_data;
    in_data.account_uid = account_token.account_uid;

    if( hb_grid_get_uid( arg_project_uid, &in_data.project_uid ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get project uid" );

        return HTTP_BADREQUEST;
    }

    if( hb_base64_decode_string( arg_code, arg_code_size, in_data.script_source, HB_DATA_MAX_SIZE, &in_data.script_source_size ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid decode base64 script source" );

        return HTTP_BADREQUEST;
    }

    hb_grid_process_lock( _args->process, account_token.account_uid );

    hb_grid_process_upload_out_data_t out_data;
    hb_result_t result = hb_grid_process_upload( _args->process, &in_data, &out_data );

    hb_grid_process_unlock( _args->process, account_token.account_uid );

    if( result == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid process" );

        return HTTP_BADREQUEST;
    }

    snprintf( _args->response, HB_GRID_RESPONSE_DATA_MAX_SIZE, "{\"code\":0}" );

    return HTTP_OK;
}