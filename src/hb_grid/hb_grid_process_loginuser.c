#include "hb_grid_process_loginuser.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_cache/hb_cache.h"
#include "hb_storage/hb_storage.h"
#include "hb_token/hb_token.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_sha1.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_process_loginuser( hb_grid_process_handle_t * _process, const hb_grid_process_loginuser_in_data_t * _in, hb_grid_process_loginuser_out_data_t * _out )
{
    hb_bool_t project_exist;
    if( hb_db_exist_project_uid( _process->db_client, _in->puid, &project_exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( project_exist == HB_FALSE )
    {
        _out->code = HB_ERROR_BAD_ARGUMENTS;

        return HB_SUCCESSFUL;
    }

    hb_db_values_handle_t * values_authentication;
    if( hb_db_create_values( &values_authentication ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_sha1_t login_sha1;
    hb_sha1( _in->login, strlen( _in->login ), &login_sha1 );

    hb_db_make_sha1_value( values_authentication, "login", HB_UNKNOWN_STRING_SIZE, &login_sha1 );

    hb_sha1_t password_sha1;
    hb_sha1( _in->password, strlen( _in->password ), &password_sha1 );

    hb_db_make_sha1_value( values_authentication, "password", HB_UNKNOWN_STRING_SIZE, &password_sha1 );

    const char * db_users_fields[] = { "_id" };
    hb_db_values_handle_t * db_users_uid_handle;

    hb_uid_t authentication_uid;
    hb_bool_t authentication_exist;
    if( hb_db_find_uid_with_values_by_name( _process->db_client, _in->puid, "users", values_authentication, &authentication_uid, db_users_fields, 1, &db_users_uid_handle, &authentication_exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( values_authentication );

    if( authentication_exist == HB_FALSE )
    {
        _out->code = HB_ERROR_NOT_FOUND;

        hb_db_destroy_values( db_users_uid_handle );

        return HB_SUCCESSFUL;
    }

    hb_uid_t uuid;
    if( hb_db_get_uid_value( db_users_uid_handle, 0, &uuid ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( db_users_uid_handle );

    _out->uuid = uuid;

    hb_user_token_t token_handle;
    token_handle.uuid = uuid;
    token_handle.puid = _in->puid;

    if( hb_token_generate( _process->cache, "UR", &token_handle, sizeof( token_handle ), 1800, &_out->token ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    _out->code = HB_ERROR_OK;

    return HB_SUCCESSFUL;
}