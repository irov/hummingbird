#include "hb_grid_process_newaccount.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_cache/hb_cache.h"
#include "hb_token/hb_token.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_sha1.h"
#include "hb_utils/hb_rand.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

hb_result_t hb_grid_process_newaccount( hb_grid_process_handle_t * _process, const hb_grid_process_newaccount_in_data_t * _in, hb_grid_process_newaccount_out_data_t * _out )
{
    HB_UNUSED( _process );

    hb_db_collection_handle_t * db_collection_accounts;
    if( hb_db_get_collection( _process->db_client, "hb", "accounts", &db_collection_accounts ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_values_handle_t * values_authentication;
    if( hb_db_create_values( &values_authentication ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_sha1_t login_sha1;
    hb_sha1( _in->login, strlen( _in->login ), &login_sha1 );

    hb_db_make_sha1_value( values_authentication, "login", HB_UNKNOWN_STRING_SIZE, &login_sha1 );

    hb_uid_t authentication_uid;
    hb_bool_t authentication_exist;
    if( hb_db_find_uid( db_collection_accounts, values_authentication, &authentication_uid, &authentication_exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( values_authentication );

    if( authentication_exist == HB_TRUE )
    {
        hb_db_destroy_collection( db_collection_accounts );

        _out->exist = HB_TRUE;

        return HB_SUCCESSFUL;
    }

    _out->exist = HB_FALSE;

    hb_sha1_t password_sha1;
    hb_sha1( _in->password, strlen( _in->password ), &password_sha1 );

    hb_db_values_handle_t * values_new;
    if( hb_db_create_values( &values_new ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_sha1_value( values_new, "login", HB_UNKNOWN_STRING_SIZE, &login_sha1 );
    hb_db_make_sha1_value( values_new, "password", HB_UNKNOWN_STRING_SIZE, &password_sha1 );

    hb_uid_t account_uid;
    if( hb_db_new_document( db_collection_accounts, values_new, &account_uid ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( values_new );

    hb_db_destroy_collection( db_collection_accounts );

    hb_account_token_t token_handle;
    token_handle.auid = account_uid;

    if( hb_token_generate( _process->cache, "AR", &token_handle, sizeof( token_handle ), 1800, &_out->token ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }    

    return HB_SUCCESSFUL;
}