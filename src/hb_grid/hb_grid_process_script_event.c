#include "hb_grid_process_script_event.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_script/hb_script.h"
#include "hb_script/hb_script_compiler.h"
#include "hb_cache/hb_cache.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_httpopt.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_process_api( hb_grid_process_handle_t * _process, const hb_grid_process_api_in_data_t * _in, hb_grid_process_api_out_data_t * _out )
{
    if( hb_cache_expire_value( _in->token, sizeof( _in->token ), 1800 ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_user_token_handle_t token_handle;
    if( hb_cache_get_value( _in->token, sizeof( _in->token ), &token_handle, sizeof( token_handle ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_script_handle_t * script_handle;
    if( hb_script_initialize( _process->db_client, HB_DATA_MAX_SIZE, HB_DATA_MAX_SIZE, token_handle.poid, token_handle.uoid, _process->matching, &script_handle ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "node", "invalid initialize script" );

        return HB_FAILURE;
    }

    if( hb_script_event_call( script_handle, _in->method, _in->data, _in->data_size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    _out->successful = HB_TRUE;
    _out->response_size = 0;
    _out->method_found = HB_TRUE;

    hb_script_stat_t stat;
    hb_script_stat( script_handle, &stat );

    _out->memory_used = stat.memory_used;
    _out->call_used = stat.call_used;

    hb_script_finalize( script_handle );

    return HB_SUCCESSFUL;
}