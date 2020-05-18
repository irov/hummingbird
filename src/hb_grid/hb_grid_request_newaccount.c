#include "hb_grid.h"
#include "hb_grid_process_newaccount.h"

#include "hb_db/hb_db.h"
#include "hb_token/hb_token.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_base16.h"
#include "hb_utils/hb_sha1.h"
#include "hb_utils/hb_oid.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
int hb_grid_request_newaccount( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size )
{
    hb_bool_t required_successful = HB_TRUE;

    hb_grid_process_newaccount_in_data_t in_data;

    {
        hb_json_handle_t * json_handle;
        if( hb_http_get_request_json( _request, &json_handle ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_copy_field_string_required( json_handle, "login", in_data.login, 128, &required_successful ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        if( hb_json_copy_field_string_required( json_handle, "password", in_data.password, 128, &required_successful ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        hb_json_destroy( json_handle );
    }

    if( required_successful == HB_FALSE )
    {
        return HTTP_BADREQUEST;
    }

    hb_grid_process_newaccount_out_data_t out_data;
    if( hb_grid_process_newaccount( _process, &in_data, &out_data ) == HB_FAILURE )
    {
        return HTTP_BADREQUEST;
    }

    if( out_data.exist == HB_FALSE )
    {
        hb_token16_t token16;
        if( hb_token_base16_encode( &out_data.token, &token16 ) == HB_FAILURE )
        {
            return HTTP_BADREQUEST;
        }

        size_t response_data_size = sprintf( _response, "{\"code\":0,\"token\":\"%.*s\"}"
            , (int)sizeof( token16 )
            , token16.value
        );

        *_size = response_data_size;
    }
    else
    {
        size_t response_data_size = sprintf( _response, "{\"code\":1,\"reason\":\"already exist\"}" );

        *_size = response_data_size;
    }

    return HTTP_OK;
}