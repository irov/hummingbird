#include "hb_node_newproject.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_cache/hb_cache.h"
#include "hb_storage/hb_storage.h"
#include "hb_sharedmemory/hb_sharedmemory.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_httpopt.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_oid.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
uint32_t hb_node_components_enumerator = e_hb_component_cache | e_hb_component_db;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_node_process( const void * _data, void * _out, size_t * _size )
{
    const hb_node_newproject_in_t * in_data = (const hb_node_newproject_in_t *)_data;
    hb_node_newproject_out_t * out_data = (hb_node_newproject_out_t *)_out;
    *_size = sizeof( hb_node_newproject_out_t );

    if( hb_cache_expire_value( in_data->token, sizeof( in_data->token ), 1800 ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_account_token_handle_t token_handle;
    if( hb_cache_get_value( in_data->token, sizeof( in_data->token ), &token_handle, sizeof( token_handle ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_collection_handle_t * db_collection_projects;
    if( hb_db_get_collection( "hb", "hb_projects", &db_collection_projects ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_value_handle_t project_values[4];
    hb_db_make_oid_value( "aoid", HB_UNKNOWN_STRING_SIZE, token_handle.aoid, project_values + 0 );
    hb_db_make_int64_value( "script_revision", HB_UNKNOWN_STRING_SIZE, 0, project_values + 1 );
    hb_db_make_int64_value( "script_version", HB_UNKNOWN_STRING_SIZE, 0, project_values + 2 );
    hb_db_make_symbol_value( "public_data", HB_UNKNOWN_STRING_SIZE, "{}", HB_UNKNOWN_STRING_SIZE, project_values + 3 );

    hb_oid_t project_oid;
    if( hb_db_new_document( db_collection_projects, project_values, 4, &project_oid ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_pid_t pid;
    uint32_t founds = 0;
    for( ; founds != 1; )
    {
        pid = hb_rand_time();
        pid &= 0x7fffffff;

        hb_db_value_handle_t handles[1];
        hb_db_make_int32_value( "pid", HB_UNKNOWN_STRING_SIZE, pid, handles + 0 );

        if( hb_db_update_values( db_collection_projects, project_oid, handles, 1 ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
        
        if( hb_db_count_values( db_collection_projects, handles, 1, &founds ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }

    hb_db_destroy_collection( db_collection_projects );

    out_data->pid = pid;

    return HB_SUCCESSFUL;
}