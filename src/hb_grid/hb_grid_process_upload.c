#include "hb_grid_process_upload.h"

#include "hb_log/hb_log.h"
#include "hb_log_tcp/hb_log_tcp.h"
#include "hb_db/hb_db.h"
#include "hb_cache/hb_cache.h"
#include "hb_script/hb_script.h"
#include "hb_script/hb_script_compiler.h"
#include "hb_storage/hb_storage.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_httpopt.h"
#include "hb_utils/hb_memmem.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

hb_result_t hb_grid_process_upload( hb_grid_process_handle_t * _process, const hb_grid_process_upload_in_data_t * _in, hb_grid_process_upload_out_data_t * _out )
{
    HB_UNUSED( _process );

    if( hb_cache_expire_value( _in->token, sizeof( _in->token ), 1800 ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_account_token_handle_t token_handle;
    if( hb_cache_get_value( _in->token, sizeof( _in->token ), &token_handle, sizeof( token_handle ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    size_t code_size;
    hb_data_t code_buffer;
    if( hb_script_compiler( _in->script_source, _in->script_source_size, code_buffer, HB_DATA_MAX_SIZE, &code_size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_sha1_t sha1;
    if( hb_storage_set( code_buffer, code_size, _in->script_source, _in->script_source_size, &sha1 ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_collection_handle_t * db_collection_projects;
    hb_db_get_collection( "hb", "hb_projects", &db_collection_projects );

    hb_db_value_handle_t project_handles[2];
    hb_db_make_oid_value( "aoid", HB_UNKNOWN_STRING_SIZE, token_handle.aoid, project_handles + 0 );
    hb_db_make_int32_value( "pid", HB_UNKNOWN_STRING_SIZE, _in->pid, project_handles + 1 );

    hb_oid_t project_oid;
    hb_bool_t project_exist;
    if( hb_db_find_oid( db_collection_projects, project_handles, 2, &project_oid, &project_exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( project_exist == HB_FALSE )
    {
        return HB_FAILURE;
    }

    hb_db_value_handle_t db_script_revision_handle[1];

    {
        const char * db_projects_fields[] = {"script_revision"};

        if( hb_db_get_values( db_collection_projects, project_oid, db_projects_fields, db_script_revision_handle, 1 ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }

    int64_t script_revision = db_script_revision_handle[0].u.i64;

    hb_db_destroy_values( db_script_revision_handle, 1 );

    hb_db_collection_handle_t * db_collection_projects_subversion;
    if( hb_db_get_collection( "hb", "hb_projects_subversion", &db_collection_projects_subversion ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_oid_t project_subversion_oid;

    if( script_revision == 0 )
    {
        hb_db_value_handle_t handles[1];
        hb_db_make_binary_value( "sha1", HB_UNKNOWN_STRING_SIZE, sha1, sizeof( sha1 ), handles + 0 );

        hb_db_new_document( db_collection_projects_subversion, handles, 1, &project_subversion_oid );
    }
    else
    {
        const char * db_projects_fields[] = {"script_sha1"};

        hb_db_value_handle_t db_script_sha1_handle[1];
        if( hb_db_get_values( db_collection_projects, project_oid, db_projects_fields, db_script_sha1_handle, 1 ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_value_handle_t handles[2];
        hb_db_make_binary_value( "sha1", HB_UNKNOWN_STRING_SIZE, sha1, sizeof( sha1 ), handles + 0 );
        hb_db_make_binary_value( "prev", HB_UNKNOWN_STRING_SIZE, db_script_sha1_handle[0].u.binary.buffer, db_script_sha1_handle[0].u.binary.length, handles + 1 );

        hb_db_new_document( db_collection_projects_subversion, handles, 2, &project_subversion_oid );

        hb_db_destroy_values( db_script_sha1_handle, 1 );
    }

    hb_db_value_handle_t db_project_handles[3];
    hb_db_make_binary_value( "script_sha1", HB_UNKNOWN_STRING_SIZE, sha1, sizeof( sha1 ), db_project_handles + 0 );
    hb_db_make_oid_value( "script_subversion", HB_UNKNOWN_STRING_SIZE, project_subversion_oid, db_project_handles + 1 );
    hb_db_make_int64_value( "script_revision", HB_UNKNOWN_STRING_SIZE, script_revision + 1, db_project_handles + 2 );

    hb_db_update_values( db_collection_projects, project_oid, db_project_handles, 3 );

    hb_db_destroy_collection( db_collection_projects );
    hb_db_destroy_collection( db_collection_projects_subversion );

    _out->revision = script_revision;

    return HB_SUCCESSFUL;
}