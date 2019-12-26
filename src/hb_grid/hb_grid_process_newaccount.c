#include "hb_grid_process_newaccount.h"

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

hb_result_t hb_grid_process_newaccount( hb_grid_process_handle_t * _process, const hb_grid_process_newaccount_in_data_t * _in, hb_grid_process_newaccount_out_data_t * _out )
{
    HB_UNUSED( _process );

    hb_db_collection_handle_t * db_collection_accounts;
    if( hb_db_get_collection( "hb", "hb_accounts", &db_collection_accounts ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_value_handle_t authentication_handles[1];

    hb_sha1_t login_sha1;
    hb_sha1( _in->login, strlen( _in->login ), &login_sha1 );

    hb_db_make_binary_value( "login", HB_UNKNOWN_STRING_SIZE, login_sha1, 20, authentication_handles + 0 );

    hb_oid_t authentication_oid;
    hb_bool_t authentication_exist;
    if( hb_db_find_oid( db_collection_accounts, authentication_handles, 1, &authentication_oid, &authentication_exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( authentication_exist == HB_TRUE )
    {
        hb_db_destroy_collection( db_collection_accounts );

        _out->exist = HB_TRUE;

        return HB_SUCCESSFUL;
    }

    _out->exist = HB_FALSE;

    hb_sha1_t password_sha1;
    hb_sha1( _in->password, strlen( _in->password ), &password_sha1 );

    hb_db_value_handle_t values[2];
    hb_db_make_binary_value( "login", HB_UNKNOWN_STRING_SIZE, login_sha1, 20, values + 0 );
    hb_db_make_binary_value( "password", HB_UNKNOWN_STRING_SIZE, password_sha1, 20, values + 1 );

    hb_oid_t account_oid;
    if( hb_db_new_document( db_collection_accounts, values, 2, &account_oid ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_account_token_handle_t token_handle;
    hb_oid_copy( token_handle.aoid, account_oid );

    if( hb_token_generate( "AR", &token_handle, sizeof( token_handle ), 1800, &_out->token ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_collection( db_collection_accounts );

    return HB_SUCCESSFUL;
}