#include "hb_messages.h"

#include "hb_mutex/hb_mutex.h"
#include "hb_memory/hb_memory.h"
#include "hb_log/hb_log.h"
#include "hb_utils/hb_hashtable.h"
#include "hb_utils/hb_list.h"
#include "hb_utils/hb_base16.h"

#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_messages_handle_t
{
    hb_mutex_handle_t * mutex;
    hb_hashtable_t * ht_channels;
} hb_messages_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_messages_channel_key_t
{
    hb_uid_t puid;
    hb_uid_t muid;    
} hb_messages_channel_key_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_messages_channel_post_handle_t
{
    hb_list_element_t element;

    uint32_t postid;
    hb_uid_t uuid;
    char message[256];
    char metainfo[256];
} hb_messages_channel_post_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_messages_channel_handle_t
{
    hb_mutex_handle_t * mutex;

    uint32_t enumerator;
    uint32_t maxpost;

    hb_list_t * l_posts;
} hb_messages_channel_handle_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_messages_create( hb_messages_handle_t ** _handle )
{
    hb_messages_handle_t * handle = HB_NEW( hb_messages_handle_t );

    hb_hashtable_t * ht_channel;
    if( hb_hashtable_create( 1024, &ht_channel ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    handle->ht_channels = ht_channel;

    hb_mutex_handle_t * mutex;
    if( hb_mutex_create( &mutex ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    handle->mutex = mutex;

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_messages_destroy( hb_messages_handle_t * _handle )
{
    hb_hashtable_destroy( _handle->ht_channels );
    hb_mutex_destroy( _handle->mutex );

    HB_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_messages_new_channel( hb_messages_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, uint32_t _maxpost, hb_uid_t * _cuid )
{
    HB_UNUSED( _handle );

    hb_db_values_handle_t * new_values;
    if( hb_db_create_values( &new_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_uid_value( new_values, "puid", HB_UNKNOWN_STRING_SIZE, _puid );
    hb_db_make_int32_value( new_values, "maxpost", HB_UNKNOWN_STRING_SIZE, _maxpost );

    hb_uid_t cuid;
    if( hb_db_new_document_by_name( _client, "hb_messages", new_values, &cuid ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( new_values );

    *_cuid = cuid;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_messages_get_channel( hb_messages_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _cuid, hb_messages_channel_handle_t ** _channel )
{
    hb_messages_channel_key_t key;
    key.puid = _puid;
    key.muid = _cuid;

    hb_messages_channel_handle_t * channel_handle = (hb_messages_channel_handle_t * )hb_hashtable_find( _handle->ht_channels, &key, sizeof( hb_messages_channel_key_t ) );

    if( channel_handle == HB_NULLPTR )
    {
        hb_db_values_handle_t * find_values;
        if( hb_db_create_values( &find_values ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_make_uid_value( find_values, "_id", HB_UNKNOWN_STRING_SIZE, _cuid );
        hb_db_make_uid_value( find_values, "puid", HB_UNKNOWN_STRING_SIZE, _puid );
        
        const char * fields[] = { "maxpost" };
        hb_db_values_handle_t * fields_values;

        hb_bool_t exist;
        if( hb_db_find_oid_with_values_by_name( _client, "hb_messages", find_values, HB_NULLPTR, fields, sizeof( fields ) / sizeof( fields[0] ), &fields_values, &exist ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        int32_t maxpost;
        if( hb_db_get_int32_value( fields_values, 0, &maxpost ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_destroy_values( fields_values );
        hb_db_destroy_values( find_values );

        if( exist == HB_FALSE )
        {
            *_channel = HB_NULLPTR;

            return HB_SUCCESSFUL;
        }

        channel_handle = HB_NEW( hb_messages_channel_handle_t );

        hb_list_t * l;
        if( hb_list_create( &l ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        channel_handle->maxpost = (uint32_t)maxpost;
        channel_handle->enumerator = 0;
        channel_handle->l_posts = l;

        hb_mutex_handle_t * mutex;
        if( hb_mutex_create( &mutex ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        channel_handle->mutex = mutex;

        if( hb_hashtable_emplace( _handle->ht_channels, &key, sizeof( hb_messages_channel_key_t ), channel_handle ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }

    *_channel = channel_handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_messages_channel_new_post( hb_messages_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _cuid, const hb_messages_post_t * _post, uint32_t * _postid, hb_error_code_t * _code )
{
    hb_mutex_lock( _handle->mutex );

    hb_messages_channel_handle_t * channel_handle;
    if( __hb_messages_get_channel( _handle, _client, _puid, _cuid, &channel_handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_mutex_unlock( _handle->mutex );

    if( channel_handle == HB_NULLPTR )
    {
        *_code = HB_ERROR_NOT_FOUND;

        return HB_SUCCESSFUL;
    }

    hb_mutex_lock( channel_handle->mutex );

    uint32_t newid = ++channel_handle->enumerator;

    hb_messages_channel_post_handle_t * post = HB_NEW( hb_messages_channel_post_handle_t );
    post->postid = newid;
    post->uuid = _post->uuid;
    strcpy( post->message, _post->message );
    strcpy( post->metainfo, _post->metainfo );

    hb_list_push_front( channel_handle->l_posts, &post->element );

    if( hb_list_count( channel_handle->l_posts ) == channel_handle->maxpost )
    {
        hb_list_element_t * oldpost_element;
        hb_list_pop_back( channel_handle->l_posts, &oldpost_element );
        hb_messages_channel_post_handle_t * oldpost = (hb_messages_channel_post_handle_t *)oldpost_element;

        HB_DELETE( oldpost );
    }

    hb_mutex_unlock( channel_handle->mutex );

    *_postid = newid;
    *_code = HB_ERROR_OK;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_messages_channel_get_posts( hb_messages_handle_t * _handle, hb_uid_t _puid, hb_uid_t _cuid, uint32_t _postid, hb_messages_get_t * _post, size_t _postcapacity, uint32_t * _count, hb_error_code_t * _code )
{
    hb_messages_channel_key_t key;
    key.puid = _puid;
    key.muid = _cuid;

    hb_mutex_lock( _handle->mutex );

    hb_messages_channel_handle_t * channel_handle = (hb_messages_channel_handle_t *)hb_hashtable_find( _handle->ht_channels, &key, sizeof( hb_messages_channel_key_t ) );

    hb_mutex_unlock( _handle->mutex );

    if( channel_handle == HB_NULLPTR )
    {
        *_code = HB_ERROR_NOT_FOUND;

        return HB_SUCCESSFUL;
    }

    *_code = HB_ERROR_OK;

    uint32_t count = 0;

    hb_mutex_lock( _handle->mutex );

    for( hb_list_element_t * element = hb_list_get_begin( channel_handle->l_posts );
        element != HB_NULLPTR;
        element = element->next )
    {
        hb_messages_channel_post_handle_t * post = (hb_messages_channel_post_handle_t *)element;

        if( post->postid == _postid )
        {
            break;
        }

        if( post->postid < _postid )
        {            
            *_code = HB_ERROR_BAD_ARGUMENTS;

            break;
        }

        if( _postcapacity-- == 0 )
        {
            break;
        }

        hb_messages_get_t * p = _post++;
        p->postid = post->postid;
        p->uuid = post->uuid;
        p->message = post->message;
        p->metainfo = post->metainfo;

        ++count;
    }

    hb_mutex_unlock( _handle->mutex );

    *_count = count;

    return HB_SUCCESSFUL;
}