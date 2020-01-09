#include "hb_grid_process_newuser.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_cache/hb_cache.h"
#include "hb_token/hb_token.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_sha1.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_oid.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

hb_result_t hb_grid_process_newuser( hb_grid_process_handle_t * _process, const hb_grid_process_newuser_in_data_t * _in, hb_grid_process_newuser_out_data_t * _out )
{
    HB_UNUSED( _process );

    hb_db_collection_handle_t * db_collection_projects;
    if( hb_db_get_collection( "hb", "hb_projects", &db_collection_projects ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_value_handle_t project_handles[1];
    hb_db_make_int32_value( "pid", HB_UNKNOWN_STRING_SIZE, _in->pid, project_handles + 0 );

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

    hb_db_value_handle_t authentication_handles[2];
    hb_db_make_int32_value( "pid", HB_UNKNOWN_STRING_SIZE, _in->pid, authentication_handles + 0 );

    hb_sha1_t login_sha1;
    hb_sha1( _in->login, strlen( _in->login ), &login_sha1 );

    hb_db_make_binary_value( "login", HB_UNKNOWN_STRING_SIZE, login_sha1, 20, authentication_handles + 1 );

    hb_bool_t authentication_exist;
    if( hb_db_find_oid( db_collection_users, authentication_handles, 2, HB_NULLPTR, &authentication_exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( authentication_exist == HB_TRUE )
    {
        hb_db_destroy_collection( db_collection_users );

        _out->exist = HB_TRUE;

        return HB_SUCCESSFUL;
    }

    _out->exist = HB_FALSE;

    if( authentication_exist == HB_FALSE )
    {
        hb_sha1_t password_sha1;
        hb_sha1( _in->password, strlen( _in->password ), &password_sha1 );

        hb_db_value_handle_t user_values[4];
        hb_db_make_int32_value( "pid", HB_UNKNOWN_STRING_SIZE, _in->pid, user_values + 0 );
        hb_db_make_binary_value( "login", HB_UNKNOWN_STRING_SIZE, login_sha1, 20, user_values + 1 );
        hb_db_make_binary_value( "password", HB_UNKNOWN_STRING_SIZE, password_sha1, 20, user_values + 2 );
        hb_db_make_symbol_value( "public_data", HB_UNKNOWN_STRING_SIZE, "{}", HB_UNKNOWN_STRING_SIZE, user_values + 3 );

        hb_oid_t user_oid;
        if( hb_db_new_document( db_collection_users, user_values, 4, &user_oid ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_user_token_handle_t token_handle;
        hb_oid_copy( token_handle.uoid, user_oid );
        hb_oid_copy( token_handle.poid, project_oid );

        if( hb_token_generate( "UR", &token_handle, sizeof( token_handle ), 1800, &_out->token ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }

    hb_db_destroy_collection( db_collection_users );

    return HB_SUCCESSFUL;
}