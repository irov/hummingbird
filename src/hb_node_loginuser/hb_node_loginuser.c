#include "hb_node_loginuser.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_cache/hb_cache.h"
#include "hb_storage/hb_storage.h"
#include "hb_token/hb_token.h"
#include "hb_sharedmemory/hb_sharedmemory.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_sha1.h"
#include "hb_utils/hb_oid.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
uint32_t hb_node_components_enumerator = e_hb_component_cache | e_hb_component_db;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_node_process( const void * _data, void * _out, size_t * _size )
{
    const hb_node_loginuser_in_t * in_data = (const hb_node_loginuser_in_t *)_data;
    hb_node_loginuser_out_t * out_data = (hb_node_loginuser_out_t *)_out;
    *_size = sizeof( hb_node_loginuser_out_t );

    hb_db_collection_handle_t * db_collection_projects;
    hb_db_get_collection( "hb", "hb_projects", &db_collection_projects );

    hb_db_value_handle_t project_handles[1];
    hb_db_make_int32_value( "pid", HB_UNKNOWN_STRING_SIZE, in_data->pid, project_handles + 0 );

    hb_oid_t project_oid;
    hb_bool_t project_exist;
    if( hb_db_find_oid( db_collection_projects, project_handles, 1, &project_oid, &project_exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_collection( db_collection_projects );

    if( project_exist == HB_FALSE )
    {
        return HB_FAILURE;
    }

    hb_db_collection_handle_t * db_collection_users;
    if( hb_db_get_collection( "hb", "hb_users", &db_collection_users ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_value_handle_t authentication_handles[3];

    hb_db_make_int32_value( "pid", HB_UNKNOWN_STRING_SIZE, in_data->pid, authentication_handles + 0 );

    hb_sha1_t login_sha1;
    hb_sha1( in_data->login, strlen( in_data->login ), login_sha1 );

    hb_db_make_binary_value( "login", HB_UNKNOWN_STRING_SIZE, login_sha1, 20, authentication_handles + 1 );

    hb_sha1_t password_sha1;
    hb_sha1( in_data->password, strlen( in_data->password ), password_sha1 );

    hb_db_make_binary_value( "password", HB_UNKNOWN_STRING_SIZE, password_sha1, 20, authentication_handles + 2 );

    hb_oid_t authentication_oid;
    hb_bool_t authentication_exist;
    if( hb_db_find_oid( db_collection_users, authentication_handles, 3, &authentication_oid, &authentication_exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_collection( db_collection_users );

    out_data->exist = authentication_exist;

    if( authentication_exist == HB_TRUE )
    {
        hb_user_token_handle_t token_handle;
        hb_oid_copy( token_handle.uoid, authentication_oid );
        hb_oid_copy( token_handle.poid, project_oid );

        if( hb_token_generate( "UR", &token_handle, sizeof( token_handle ), 1800, out_data->token ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }
    
    return HB_SUCCESSFUL;
}