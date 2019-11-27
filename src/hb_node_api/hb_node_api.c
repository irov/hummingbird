#include "hb_node_api.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_script/hb_script.h"
#include "hb_script/hb_script_compiler.h"
#include "hb_cache/hb_cache.h"
#include "hb_storage/hb_storage.h"
#include "hb_sharedmemory/hb_sharedmemory.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_httpopt.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
uint32_t hb_node_components_enumerator = e_hb_component_cache | e_hb_component_db | e_hb_component_storage;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static hb_result_t __node_initialize_script( const hb_token_handle_t _token )
{
    if( hb_script_initialize( HB_DATA_MAX_SIZE, HB_DATA_MAX_SIZE, _token.uoid, _token.poid ) == HB_FAILURE )
    {
        hb_log_message( "node", HB_LOG_ERROR, "invalid initialize script" );

        return HB_FAILURE;
    }

    hb_db_collection_handle_t * db_collection_projects;
    if( hb_db_get_collection( "hb", "hb_projects", &db_collection_projects ) == HB_FAILURE )
    {
        hb_log_message( "script", HB_LOG_ERROR, "invalid initialize script: db not found collection 'hb_projects'" );

        return HB_FAILURE;
    }

    const char * db_projects_fields[] = { "script_sha1" };

    hb_db_value_handle_t db_script_sha1_handles[1];
    if( hb_db_get_values( db_collection_projects, _token.poid, db_projects_fields, 1, db_script_sha1_handles ) == HB_FAILURE )
    {
        hb_log_message( "node", HB_LOG_ERROR, "invalid initialize script: collection 'hb_projects' not found 'script_sha1'" );

        return HB_FAILURE;
    }

    if( db_script_sha1_handles[0].u.binary.length != sizeof( hb_sha1_t ) )
    {
        hb_log_message( "node", HB_LOG_ERROR, "invalid initialize script: invalid data 'script_sha1'" );

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
        hb_log_message( "node", HB_LOG_ERROR, "invalid initialize script: invalid get data from storage" );

        return HB_FAILURE;
    }

    if( hb_script_load( script_data, script_data_size ) == HB_FAILURE )
    {
        hb_log_message( "node", HB_LOG_ERROR, "invalid initialize script: invalid load data" );

        return HB_FAILURE;
    }    

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_node_process( const void * _data, void * _out, size_t * _size )
{
    const hb_node_api_in_t * in_data = (const hb_node_api_in_t *)_data;
    hb_node_api_out_t * out_data = (hb_node_api_out_t *)_out;
    *_size = sizeof( hb_node_api_out_t );

    if( hb_cache_available() == HB_FALSE )
    {
        return HB_FAILURE;
    }
    
    if( hb_cache_expire_value( in_data->token, sizeof( in_data->token ), 1800 ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_token_handle_t token_handle;
    if( hb_cache_get_value( in_data->token, sizeof( in_data->token ), &token_handle, sizeof( token_handle ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( __node_initialize_script( token_handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    switch( in_data->category )
    {
    case e_hb_node_api:
        {
            hb_error_code_t code;
            hb_result_t result = hb_script_server_call( in_data->method, in_data->data, in_data->data_size, out_data->response_data, HB_GRID_REQUEST_DATA_MAX_SIZE, &out_data->response_size, &out_data->successful, &code );

            if( result == HB_FAILURE )
            {
                if( code == HB_ERROR_NOT_FOUND )
                {
                    out_data->response_size = 0;
                    out_data->successful = HB_FALSE;
                    out_data->method_found = HB_FALSE;
                }
                else
                {
                    return HB_FAILURE;
                }
            }
            else
            {
                out_data->method_found = HB_TRUE;
            }            
        }break;
    case e_hb_node_event:
        {
            if( hb_script_event_call( in_data->method, in_data->data, in_data->data_size ) == HB_FAILURE )
            {
                return HB_FAILURE;
            }

            out_data->successful = HB_TRUE;
            out_data->response_size = 0;
            out_data->method_found = HB_TRUE;
        }
    }

    hb_script_stat_t stat;
    hb_script_stat( &stat );

    out_data->memory_used = stat.memory_used;
    out_data->call_used = stat.call_used;

    hb_script_finalize();

    return EXIT_SUCCESS;
}