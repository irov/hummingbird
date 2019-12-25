#include "hb_grid_process_api.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_script/hb_script.h"
#include "hb_script/hb_script_compiler.h"
#include "hb_cache/hb_cache.h"
#include "hb_storage/hb_storage.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_httpopt.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
static hb_result_t __grid_process_api_initialize_script( const hb_user_token_handle_t * _token )
{
    if( hb_script_initialize( HB_DATA_MAX_SIZE, HB_DATA_MAX_SIZE, _token->uoid, _token->poid ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "node", "invalid initialize script" );

        return HB_FAILURE;
    }

    hb_db_collection_handle_t * db_collection_projects;
    if( hb_db_get_collection( "hb", "hb_projects", &db_collection_projects ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "hb_projects" 
        );

        return HB_FAILURE;
    }

    const char * db_projects_fields[] = { "script_sha1" };

    hb_db_value_handle_t db_script_sha1_handles[1];
    if( hb_db_get_values( db_collection_projects, _token->poid, db_projects_fields, db_script_sha1_handles, 1 ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "node", "invalid initialize script: collection '%s' not found 'script_sha1'"
            , "hb_projects" 
        );

        return HB_FAILURE;
    }

    if( db_script_sha1_handles[0].u.binary.length != sizeof( hb_sha1_t ) )
    {
        HB_LOG_MESSAGE_ERROR( "node", "invalid initialize script: collection '%s' invalid data 'script_sha1'"
            , "hb_projects"
        );

        return HB_FAILURE;
    }

    hb_sha1_t script_sha1;
    memcpy( script_sha1, db_script_sha1_handles[0].u.binary.buffer, sizeof( hb_sha1_t ) );

    hb_db_destroy_values( db_script_sha1_handles, 1 );
    hb_db_destroy_collection( db_collection_projects );

    size_t script_data_size;
    hb_data_t script_data;
    if( hb_storage_get_code( script_sha1, script_data, sizeof( script_data ), &script_data_size ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "node", "invalid initialize script: collection '%s' invalid get data from storage"
            , "hb_projects"
        );

        return HB_FAILURE;
    }

    if( hb_script_load( script_data, script_data_size ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "node", "invalid initialize script: collection '%s' invalid load data"
            , "hb_projects"
        );

        return HB_FAILURE;
    }    

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_process_api( const hb_grid_process_api_in_data_t * _in, hb_grid_process_api_out_data_t * _out )
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

    if( __grid_process_api_initialize_script( &token_handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    switch( _in->category )
    {
    case e_hb_node_api:
        {
            hb_error_code_t code;
            hb_result_t result = hb_script_server_call( _in->method, _in->data, _in->data_size, _out->response_data, HB_GRID_REQUEST_DATA_MAX_SIZE, &_out->response_size, &_out->successful, &code );

            if( result == HB_FAILURE )
            {
                if( code == HB_ERROR_NOT_FOUND )
                {
                    _out->response_size = 0;
                    _out->successful = HB_TRUE;
                    _out->method_found = HB_FALSE;
                }
                else
                {
                    _out->response_size = 0;
                    _out->successful = HB_FALSE;
                    _out->method_found = HB_FALSE;
                }
            }
            else
            {
                _out->successful = HB_TRUE;
                _out->method_found = HB_TRUE;
            }
        }break;
    case e_hb_node_event:
        {
            if( hb_script_event_call( _in->method, _in->data, _in->data_size ) == HB_FAILURE )
            {
                return HB_FAILURE;
            }

            _out->successful = HB_TRUE;
            _out->response_size = 0;
            _out->method_found = HB_TRUE;
        }break;
    default:
        {
            return HB_FAILURE;
        }break;
    }

    hb_script_stat_t stat;
    hb_script_stat( &stat );

    _out->memory_used = stat.memory_used;
    _out->call_used = stat.call_used;

    hb_script_finalize();

    return HB_SUCCESSFUL;
}