#include "hb_node_loginuser.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_cache/hb_cache.h"
#include "hb_storage/hb_storage.h"
#include "hb_token/hb_token.h"
#include "hb_sharedmemory/hb_sharedmemory.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_sha1.h"
#include "hb_utils/hb_time.h"
#include "hb_utils/hb_oid.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Windows.h>

//////////////////////////////////////////////////////////////////////////
static void __hb_log_observer( const char * _category, int _level, const char * _message )
{    
    const char * ls[] = {"info", "warning", "error", "critical"};

    printf( "[%s] %s: %s\n", _category, ls[_level], _message );
}
//////////////////////////////////////////////////////////////////////////
int main( int _argc, char * _argv[] )
{
    HB_UNUSED( _argc );
    HB_UNUSED( _argv );

    MessageBox( NULL, "Test", "Test", MB_OK );

    hb_log_initialize();
    if( hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_observer ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    const char * sm_name;
    if( hb_getopt( _argc, _argv, "--sm", &sm_name ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_sharedmemory_handle_t sharedmemory_handle;
    if( hb_sharedmemory_open( sm_name, 65536, &sharedmemory_handle ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_node_loginuser_in_t in_data;
    if( hb_sharedmemory_read( &sharedmemory_handle, &in_data, sizeof( in_data ), HB_NULLPTR ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( in_data.magic_number != hb_node_loginuser_magic_number )
    {
        return EXIT_FAILURE;
    }

    if( in_data.version_number != hb_node_loginuser_version_number )
    {
        return EXIT_FAILURE;
    }

    if( hb_cache_initialize( in_data.cache_uri, in_data.cache_port ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( hb_db_initialze( "hb_node_loginuser", in_data.db_uri ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_db_collection_handle_t db_projects_handle;
    hb_db_get_collection( "hb", "hb_projects", &db_projects_handle );

    hb_db_value_handle_t project_handles[1];

    hb_db_make_int32_value( "pid", ~0U, in_data.pid, project_handles + 0 );

    hb_oid_t puid;
    hb_result_t project_exist;
    if( hb_db_find_oid( &db_projects_handle, project_handles, 1, puid, &project_exist ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_db_collection_handle_t db_users_handle;
    if( hb_db_get_collection( "hb", "hb_users", &db_users_handle ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_db_value_handle_t authentication_handles[3];

    hb_db_make_int32_value( "pid", ~0U, in_data.pid, authentication_handles + 0 );

    hb_sha1_t login_sha1;
    hb_sha1( in_data.login, strlen( in_data.login ), login_sha1 );

    hb_db_make_binary_value( "login", ~0U, login_sha1, 20, authentication_handles + 1 );

    hb_sha1_t password_sha1;
    hb_sha1( in_data.password, strlen( in_data.password ), password_sha1 );

    hb_db_make_binary_value( "password", ~0U, password_sha1, 20, authentication_handles + 2 );

    hb_oid_t authentication_oid;
    hb_result_t authentication_exist;
    if( hb_db_find_oid( &db_users_handle, authentication_handles, 3, authentication_oid, &authentication_exist ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_node_loginuser_out_t out_data;
    out_data.magic_number = hb_node_loginuser_magic_number;
    out_data.version_number = hb_node_loginuser_version_number;
    out_data.exist = authentication_exist;

    if( authentication_exist == HB_SUCCESSFUL )
    {
        hb_db_value_handle_t user_handles[3];
        hb_db_make_int32_value( "pid", ~0U, in_data.pid, user_handles + 0 );
        hb_db_make_binary_value( "login", ~0U, login_sha1, 20, user_handles + 1 );
        hb_db_make_binary_value( "password", ~0U, password_sha1, 20, user_handles + 2 );

        hb_oid_t user_oid;
        hb_db_new_document( &db_users_handle, user_handles, 3, user_oid );

        hb_token_handle_t token_handle;
        hb_oid_copy( token_handle.uoid, user_oid );
        hb_oid_copy( token_handle.poid, puid );

        hb_token_t token;
        hb_token_generate( "user_token", &token_handle, sizeof( token_handle ), 1800, token );

        hb_token_copy( out_data.token, token );
    }
    
    hb_cache_finalize();
    hb_db_finalize();

    hb_sharedmemory_rewind( &sharedmemory_handle );
    hb_sharedmemory_write( &sharedmemory_handle, &out_data, sizeof( out_data ) );
    hb_sharedmemory_destroy( &sharedmemory_handle );

    hb_log_finalize();

    return EXIT_SUCCESS;
}