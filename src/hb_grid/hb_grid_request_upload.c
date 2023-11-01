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
    hb_bool_t required = HB_TRUE;

    const char * arg_account_token;
    hb_json_get_field_string_required( _args->data, "account_token", &arg_account_token, HB_NULLPTR, &required );

    const char * arg_project_uid;
    hb_json_get_field_string_required( _args->data, "project_uid", &arg_project_uid, HB_NULLPTR, &required );

    const char * arg_code;
    hb_size_t arg_code_size;
    hb_json_get_field_string_required( _args->data, "code", &arg_code, &arg_code_size, &required );

    if( required == HB_FALSE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid required args" );

        return HTTP_BADREQUEST;
    }

    hb_account_token_t account_token;
    if( hb_cache_get_token( _args->process->cache, arg_account_token, 1800, &account_token, sizeof( account_token ), HB_NULLPTR ) == HB_FAILURE )
    {
        snprintf( _args->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get account token" );

        return HTTP_BADREQUEST;
    }

    hb_grid_process_upload_in_data_t in_data;
    in_data.account_uid = account_token.account_uid;

    if( hb_base16_decode( arg_project_uid, HB_UNKNOWN_STRING_SIZE, &in_data.project_uid, sizeof( in_data.project_uid ), HB_NULLPTR ) == HB_FAILURE )
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