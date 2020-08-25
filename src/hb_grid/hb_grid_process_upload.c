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

    if( hb_cache_expire_value( _process->cache, _in->token.value, sizeof( _in->token ), 1800 ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_account_token_t token_handle;
    if( hb_cache_get_value( _process->cache, _in->token.value, sizeof( _in->token ), &token_handle, sizeof( token_handle ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    size_t code_size;
    hb_data_t code_buffer;
    if( hb_script_compiler( _in->script_source, _in->script_source_size, code_buffer, HB_DATA_MAX_SIZE, &code_size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_collection_handle_t * db_collection_scripts;
    if( hb_db_get_project_collection( _process->db_client, _in->puid, "scripts", &db_collection_scripts ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_sha1_t sha1;
    if( hb_storage_set( db_collection_scripts, code_buffer, code_size, _in->script_source, _in->script_source_size, &sha1 ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_collection( db_collection_scripts );

    hb_db_collection_handle_t * db_collection_projects;
    if( hb_db_get_collection( _process->db_client, "hb", "projects", &db_collection_projects ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_values_handle_t * values_project_found;
    if( hb_db_create_values( &values_project_found ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_uid_value( values_project_found, "_id", HB_UNKNOWN_STRING_SIZE, _in->puid );
    hb_db_make_uid_value( values_project_found, "aoid", HB_UNKNOWN_STRING_SIZE, token_handle.auid );    

    hb_uid_t project_uid;
    hb_bool_t project_exist;
    if( hb_db_find_uid( db_collection_projects, values_project_found, &project_uid, &project_exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( values_project_found );

    if( project_exist == HB_FALSE )
    {
        return HB_FAILURE;
    }

    hb_db_values_handle_t * db_script_revision_handle;

    {
        const char * db_projects_fields[] = {"script_revision"};

        if( hb_db_get_values( db_collection_projects, project_uid, db_projects_fields, 1, &db_script_revision_handle, HB_NULLPTR ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }

    int32_t script_revision;
    if( hb_db_get_int32_value( db_script_revision_handle, 0, &script_revision ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( db_script_revision_handle );

    hb_db_collection_handle_t * db_collection_source_subversion;
    if( hb_db_get_project_collection( _process->db_client, _in->puid, "source_subversion", &db_collection_source_subversion ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_uid_t project_subversion_uid;

    if( script_revision == 0 )
    {
        hb_db_values_handle_t * values_new;
        if( hb_db_create_values( &values_new ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_make_sha1_value( values_new, "sha1", HB_UNKNOWN_STRING_SIZE, &sha1 );

        if( hb_db_new_document( db_collection_source_subversion, values_new, &project_subversion_uid ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_destroy_values( values_new );
    }
    else
    {
        const char * db_projects_fields[] = {"script_sha1"};

        hb_db_values_handle_t * values_sha1;
        if( hb_db_get_values( db_collection_projects, project_uid, db_projects_fields, 1, &values_sha1, HB_NULLPTR ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        const void * script_sha1_binary;
        size_t script_sha1_binary_length;
        if( hb_db_get_binary_value( values_sha1, 0, &script_sha1_binary, &script_sha1_binary_length ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_values_handle_t * values_new;
        if( hb_db_create_values( &values_new ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_make_sha1_value( values_new, "sha1", HB_UNKNOWN_STRING_SIZE, &sha1 );
        hb_db_make_binary_value( values_new, "prev", HB_UNKNOWN_STRING_SIZE, script_sha1_binary, script_sha1_binary_length );

        if( hb_db_new_document( db_collection_source_subversion, values_new, &project_subversion_uid ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_destroy_values( values_sha1 );
        hb_db_destroy_values( values_new );
    }

    hb_db_values_handle_t * values_update;
    if( hb_db_create_values( &values_update ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_sha1_value( values_update, "script_sha1", HB_UNKNOWN_STRING_SIZE, &sha1 );
    hb_db_make_uid_value( values_update, "script_subversion", HB_UNKNOWN_STRING_SIZE, project_subversion_uid );
    hb_db_make_int32_value( values_update, "script_revision", HB_UNKNOWN_STRING_SIZE, script_revision + 1 );

    if( hb_db_update_values( db_collection_projects, project_uid, values_update ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( values_update );

    hb_db_destroy_collection( db_collection_projects );
    hb_db_destroy_collection( db_collection_source_subversion );

    _out->revision = script_revision;

    return HB_SUCCESSFUL;
}