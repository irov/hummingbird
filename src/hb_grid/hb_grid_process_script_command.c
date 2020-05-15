#include "hb_grid_process_script_command.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_script/hb_script.h"
#include "hb_script/hb_script_compiler.h"
#include "hb_cache/hb_cache.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_httpopt.h"
#include "hb_utils/hb_oid.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_process_script_command( hb_grid_process_handle_t * _process, const hb_grid_process_script_command_in_data_t * _in, hb_grid_process_script_command_out_data_t * _out )
{
    if( hb_cache_expire_value( _process->cache, _in->token, sizeof( _in->token ), 1800 ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_account_token_handle_t token_handle;
    if( hb_cache_get_value( _process->cache, _in->token, sizeof( _in->token ), &token_handle, sizeof( token_handle ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_values_handle_t * values_project_found;
    if( hb_db_create_values( &values_project_found ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_oid_value( values_project_found, "aoid", HB_UNKNOWN_STRING_SIZE, token_handle.aoid );
    hb_db_make_int32_value( values_project_found, "pid", HB_UNKNOWN_STRING_SIZE, _in->pid );

    hb_oid_t project_oid;
    hb_bool_t project_exist;
    if( hb_db_find_oid_by_name( _process->db_client, "hb_projects", values_project_found, &project_oid, &project_exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( values_project_found );

    hb_script_handle_t * script_handle;
    if( hb_script_initialize( _process->cache, _process->db_client, HB_DATA_MAX_SIZE, HB_DATA_MAX_SIZE, project_oid, HB_OID_NONE, _process->matching, &script_handle ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "node", "invalid initialize script" );

        return HB_FAILURE;
    }

    hb_error_code_t code;
    if( hb_script_command_call( script_handle, _in->method, _in->data, _in->data_size, _out->response_data, HB_GRID_REQUEST_DATA_MAX_SIZE, &_out->response_size, &code ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    switch( code )
    {
    case HB_ERROR_OK:
        {
            _out->successful = HB_TRUE;
            _out->method_found = HB_TRUE;
        }break;
    case HB_ERROR_INTERNAL:
        {
            _out->response_size = 0;
            _out->successful = HB_FALSE;
            _out->method_found = HB_TRUE;
        }break;
    case HB_ERROR_NOT_FOUND:
        {
            _out->response_size = 0;
            _out->successful = HB_TRUE;
            _out->method_found = HB_FALSE;
        }break;
    }
    
    hb_script_stat_t stat;
    hb_script_stat( script_handle, &stat );

    _out->memory_used = stat.memory_used;
    _out->call_used = stat.call_used;

    hb_script_finalize( script_handle );

    return HB_SUCCESSFUL;
}