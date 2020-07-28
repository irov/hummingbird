#include "hb_grid_process_script_api.h"

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
hb_result_t hb_grid_process_script_api( hb_grid_process_handle_t * _process, const hb_grid_process_script_api_in_data_t * _in, hb_grid_process_script_api_out_data_t * _out )
{
    hb_script_handle_t * script_handle;
    if( hb_script_initialize( _process->cache, _process->db_client, HB_DATA_MAX_SIZE, HB_DATA_MAX_SIZE, _in->puid, _in->uuid, _process->matching, &script_handle ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "process", "invalid initialize script" );

        return HB_FAILURE;
    }

    if( hb_script_api_call( script_handle, _in->api, _in->method, _in->json_handle, _out->response_data, HB_GRID_REQUEST_DATA_MAX_SIZE, &_out->response_size, &_out->code ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_script_stat_t stat;
    hb_script_stat( script_handle, &stat );

    _out->memory_used = stat.memory_used;
    _out->call_used = stat.call_used;

    hb_script_finalize( script_handle );

    return HB_SUCCESSFUL;
}