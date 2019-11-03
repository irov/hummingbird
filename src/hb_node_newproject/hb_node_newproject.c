#include "hb_node_newproject.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
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

#include <Windows.h>

//////////////////////////////////////////////////////////////////////////
uint32_t hb_node_components_enumerator = e_hb_component_cache | e_hb_component_db;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_node_process( const void * _data, void * _out, size_t * _size )
{
    const hb_node_newproject_in_t * in_data = (const hb_node_newproject_in_t *)_data;
    hb_node_newproject_out_t * out_data = (hb_node_newproject_out_t *)_out;
    *_size = sizeof( hb_node_newproject_out_t );

    HB_UNUSED( in_data );

    //MessageBox( NULL, "Test", "Test", MB_OK );

    hb_db_collection_handle_t db_collection_projects;
    if( hb_db_get_collection( "hb", "hb_projects", &db_collection_projects ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_value_handle_t new_value[1];
    hb_db_make_int64_value( "script_revision", ~0U, 0, new_value + 0 );

    hb_oid_t project_oid;
    if( hb_db_new_document( &db_collection_projects, new_value, 1, project_oid ) == HB_FAILURE )
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
        hb_db_make_int32_value( "pid", ~0U, pid, handles + 0 );

        if( hb_db_update_values( &db_collection_projects, project_oid, handles, 1 ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
        
        if( hb_db_count_values( &db_collection_projects, handles, 1, &founds ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }

    hb_db_destroy_collection( &db_collection_projects );

    out_data->pid = pid;

    return HB_SUCCESSFUL;
}