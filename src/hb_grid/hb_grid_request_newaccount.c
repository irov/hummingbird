#include "hb_grid.h"

#include "hb_node_newaccount/hb_node_newaccount.h"
#include "hb_node_api/hb_node_api.h"
#include "hb_db/hb_db.h"
#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_process/hb_process.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"
#include "hb_utils/hb_sha1.h"
#include "hb_utils/hb_oid.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
static hb_result_t hb_node_process2( const char * _login, const char * _password, hb_bool_t * _exist, hb_token_t * _token )
{
    hb_db_collection_handle_t * db_collection_accounts;
    if( hb_db_get_collection( "hb", "hb_accounts", &db_collection_accounts ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_value_handle_t authentication_handles[1];

    hb_sha1_t login_sha1;
    hb_sha1( _login, strlen( _login ), &login_sha1 );

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

        *_exist = HB_TRUE;

        return HB_SUCCESSFUL;
    }

    *_exist = HB_FALSE;

    hb_sha1_t password_sha1;
    hb_sha1( _password, strlen( _password ), &password_sha1 );

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

    if( hb_token_generate( "AR", &token_handle, sizeof( token_handle ), 1800, _token ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_collection( db_collection_accounts );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
int hb_grid_request_newaccount( struct evhttp_request * _request, struct hb_grid_process_handle_t * _process, char * _response, size_t * _size )
{
    HB_UNUSED( _process );

    char login[128];
    char password[128];

    {
        hb_json_handle_t * json_handle;
        if( hb_http_get_request_json( _request, &json_handle ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_copy_field_string( json_handle, "login", login, 128 ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_copy_field_string( json_handle, "password", password, 128 ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        hb_json_destroy( json_handle );
    }

    hb_bool_t exist;
    hb_token_t token;
    if( hb_node_process2( login, password, &exist, &token ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }
    
    if( exist == HB_FALSE )
    {
        hb_token16_t token16;
        hb_token_base16_encode( token, &token16 );

        size_t response_data_size = sprintf( _response, "{\"code\": 0, \"token\": \"%.*s\"}"
            , (int)sizeof( token16 )
            , token16
        );

        *_size = response_data_size;
    }
    else
    {
        size_t response_data_size = sprintf( _response, "{\"code\": 1, \"reason\": \"already exist\"}" );

        *_size = response_data_size;
    }

    return HTTP_OK;
}