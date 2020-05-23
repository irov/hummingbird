#include "hb_messages.h"

#include "hb_memory/hb_memory.h"
#include "hb_log/hb_log.h"
#include "hb_utils/hb_hashtable.h"
#include "hb_utils/hb_oid.h"
#include "hb_utils/hb_base16.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_messages_handle_t
{
    hb_db_collection_handle_t * db_messages;
    hb_hashtable_t * ht_channel;
} hb_messages_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_messages_channel_key_t
{
    hb_uid_t puid;
    hb_uid_t muid;
} hb_messages_channel_key_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_messages_channel_handle_t
{
    uint32_t dummy;
} hb_messages_channel_handle_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_messages_create( const hb_db_client_handle_t * _client, hb_messages_handle_t ** _handle )
{
    hb_messages_handle_t * handle = HB_NEW( hb_messages_handle_t );

    hb_db_collection_handle_t * db_messages;
    if( hb_db_get_collection( _client, "hb", "hb_messages", &db_messages ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "messages", "invalid get collection '%s'"
            , "hb_messages"
        );

        return HB_FAILURE;
    }

    handle->db_messages = db_messages;

    hb_hashtable_t * ht_channel;
    if( hb_hashtable_create( 1024, &ht_channel ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    handle->ht_channel = ht_channel;

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_messages_destroy( hb_messages_handle_t * _handle )
{
    hb_db_destroy_collection( _handle->db_messages );

    HB_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_messages_new_channel( hb_messages_handle_t * _handle, hb_uid_t _puid, hb_uid_t * _muid )
{
    hb_db_values_handle_t * new_values;
    if( hb_db_create_values( &new_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_uid_value( new_values, "puid", HB_UNKNOWN_STRING_SIZE, _puid );

    hb_oid_t moid;
    if( hb_db_new_document( _handle->db_messages, new_values, &moid ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_values_handle_t * uid_values;
    if( hb_db_create_values( &uid_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_uid_value( uid_values, "puid", HB_UNKNOWN_STRING_SIZE, _puid );

    hb_uid_t muid;
    if( hb_db_make_uid( _handle->db_messages, &moid, uid_values, &muid ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( uid_values );

    *_muid = muid;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_messages_get_channel( hb_messages_handle_t * _handle, hb_uid_t _puid, hb_uid_t _muid, hb_messages_channel_handle_t ** _channel )
{
    hb_messages_channel_key_t key;
    key.puid = _puid;
    key.muid = _muid;

    hb_messages_channel_handle_t * channel_handle = (hb_messages_channel_handle_t * )hb_hashtable_find( _handle->ht_channel, &key, sizeof( hb_messages_channel_key_t ) );

    if( channel_handle == HB_NULLPTR )
    {
        hb_db_values_handle_t * find_values;
        if( hb_db_create_values( &find_values ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_make_uid_value( find_values, "uid", HB_UNKNOWN_STRING_SIZE, _muid );
        hb_db_make_uid_value( find_values, "puid", HB_UNKNOWN_STRING_SIZE, _puid );
        
        hb_bool_t exist;
        if( hb_db_find_oid( _handle->db_messages, find_values, HB_NULLPTR, &exist ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_destroy_values( find_values );

        if( exist == HB_FALSE )
        {
            *_channel = HB_NULLPTR;

            return HB_SUCCESSFUL;
        }

        channel_handle = HB_NEW( hb_messages_channel_handle_t );

        if( hb_hashtable_emplace( _handle->ht_channel, &key, sizeof( hb_messages_channel_key_t ), channel_handle ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }

    *_channel = channel_handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_messages_channel_new_message( hb_messages_handle_t * _handle, hb_uid_t _puid, hb_uid_t _muid, hb_uid_t _uuid, const hb_messages_post_t * _post, hb_error_code_t * _code )
{
    HB_UNUSED( _uuid );
    HB_UNUSED( _post );

    hb_messages_channel_handle_t * channel_handle;
    if( __hb_messages_get_channel( _handle, _puid, _muid, &channel_handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( channel_handle == HB_NULLPTR )
    {
        *_code = HB_ERROR_NOT_FOUND;

        return HB_SUCCESSFUL;
    }

    *_code = HB_ERROR_OK;

    return HB_SUCCESSFUL;
}