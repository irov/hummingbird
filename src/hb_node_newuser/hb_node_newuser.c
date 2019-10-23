#include "hb_node_newuser.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_storage/hb_storage.h"
#include "hb_sharedmemory/hb_sharedmemory.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_sha1.h"
#include "hb_utils/hb_time.h"

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
    hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_observer );

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

    hb_node_newuser_in_t in_data;
    if( hb_sharedmemory_read( &sharedmemory_handle, &in_data, sizeof( in_data ), HB_NULLPTR ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( hb_db_initialze( "hb_node_newuser", in_data.db_uri ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_db_collection_handle_t db_users_handle;
    hb_db_get_collection( "hb", "hb_users", &db_users_handle );

    hb_db_value_handle_t authentication_handles[2];

    uint8_t login_sha1[20];
    hb_sha1( in_data.login, strlen( in_data.login ), login_sha1 );

    hb_db_make_binary_value( "login", ~0U, login_sha1, 20, authentication_handles + 0 );

    uint8_t password_sha1[20];
    hb_sha1( in_data.password, strlen( in_data.password ), password_sha1 );

    hb_db_make_binary_value( "password", ~0U, password_sha1, 20, authentication_handles + 1 );

    uint8_t authentication_oid[12];
    hb_result_t authentication_exist;
    if( hb_db_find_oid( &db_users_handle, authentication_handles, 2, authentication_oid, &authentication_exist ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_node_newuser_out_t out_data;
    out_data.exist = authentication_exist;

    if( authentication_exist == HB_FAILURE )
    {
        hb_db_value_handle_t user_handles[3];
        hb_db_make_oid_value( "puid", ~0U, in_data.puid, user_handles + 0 );
        hb_db_make_binary_value( "login", ~0U, login_sha1, 20, user_handles + 1 );
        hb_db_make_binary_value( "password", ~0U, password_sha1, 20, user_handles + 2 );

        uint8_t user_oid[12];
        hb_db_new_document( &db_users_handle, user_handles, 3, user_oid );

        hb_db_collection_handle_t db_token_handle;
        hb_db_get_collection( "hb", "hb_token", &db_token_handle );

        hb_db_value_handle_t token_handles[3];
        hb_db_make_oid_value( "uuid", ~0U, user_oid, token_handles + 0 );
        hb_db_make_oid_value( "puid", ~0U, in_data.puid, token_handles + 1 );

        hb_time_t t;
        hb_time( &t );
        hb_db_make_time_value( "loginAt", ~0U, t, token_handles + 2 );

        uint8_t token_oid[12];
        hb_db_new_document( &db_token_handle, token_handles, 3, token_oid );

        memcpy( out_data.token, token_oid, 12 );
    }
    
    hb_db_finalize();

    hb_sharedmemory_rewind( &sharedmemory_handle );
    hb_sharedmemory_write( &sharedmemory_handle, &out_data, sizeof( out_data ) );
    hb_sharedmemory_destroy( &sharedmemory_handle );

    hb_log_finalize();

    return EXIT_SUCCESS;
}