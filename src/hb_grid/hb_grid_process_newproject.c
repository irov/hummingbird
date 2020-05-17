#include "hb_grid_process_newproject.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_cache/hb_cache.h"
#include "hb_storage/hb_storage.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_httpopt.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_oid.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

hb_result_t hb_grid_process_newproject( hb_grid_process_handle_t * _process, const hb_grid_process_newproject_in_data_t * _in, hb_grid_process_newproject_out_data_t * _out )
{
    HB_UNUSED( _process );

    if( hb_cache_expire_value( _process->cache, _in->token.value, sizeof( _in->token ), 1800 ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_account_token_t token_handle;
    if( hb_cache_get_value( _process->cache, _in->token.value, sizeof( _in->token ), &token_handle, sizeof( token_handle ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_collection_handle_t * db_collection_projects;
    if( hb_db_get_collection( _process->db_client, "hb", "hb_projects", &db_collection_projects ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_values_handle_t * project_values;
    if( hb_db_create_values( &project_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_oid_value( project_values, "aoid", HB_UNKNOWN_STRING_SIZE, &token_handle.aoid );
    hb_db_make_int64_value( project_values, "script_revision", HB_UNKNOWN_STRING_SIZE, 0 );
    hb_db_make_int64_value( project_values, "script_version", HB_UNKNOWN_STRING_SIZE, 0 );    
    hb_db_make_symbol_value( project_values, "public_data", HB_UNKNOWN_STRING_SIZE, "{}", HB_UNKNOWN_STRING_SIZE );

    hb_oid_t project_oid;
    if( hb_db_new_document( db_collection_projects, project_values, &project_oid ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( project_values );

    hb_pid_t pid;
    if( hb_db_make_pid( db_collection_projects, &project_oid, HB_NULLPTR, &pid ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_collection( db_collection_projects );

    _out->pid = pid;

    return HB_SUCCESSFUL;
}