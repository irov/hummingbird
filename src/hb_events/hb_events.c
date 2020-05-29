#include "hb_events.h"

#include "hb_mutex/hb_mutex.h"
#include "hb_memory/hb_memory.h"
#include "hb_log/hb_log.h"
#include "hb_utils/hb_hashtable.h"
#include "hb_utils/hb_list.h"
#include "hb_utils/hb_base16.h"

#include "string.h"

//////////////////////////////////////////////////////////////////////////
typedef struct hb_events_handle_t
{
    hb_mutex_handle_t * mutex;
    hb_hashtable_t * ht_topics;
} hb_events_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_events_topic_key_t
{
    hb_uid_t puid;
    hb_uid_t tuid;
} hb_events_topic_key_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_events_topic_handle_t
{
    hb_time_t start;
    uint32_t delay;
    uint32_t index;
    char name[32];
    char message[HB_DATA_MAX_SIZE];

    hb_mutex_handle_t * mutex;
} hb_events_topic_handle_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_events_create( hb_events_handle_t ** _handle )
{
    hb_events_handle_t * handle = HB_NEW( hb_events_handle_t );

    hb_hashtable_t * ht_topics;
    if( hb_hashtable_create( 1024, &ht_topics ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    handle->ht_topics = ht_topics;

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
void hb_messages_destroy( hb_events_handle_t * _handle )
{
    hb_hashtable_destroy( _handle->ht_topics );
    hb_mutex_destroy( _handle->mutex );

    HB_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_events_new_topic( hb_events_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, const char * _name, uint32_t _delay, hb_uid_t * _tuid )
{
    HB_UNUSED( _handle );

    hb_db_values_handle_t * new_values;
    if( hb_db_create_values( &new_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_uid_value( new_values, "puid", HB_UNKNOWN_STRING_SIZE, _puid );
    hb_db_make_string_value( new_values, "name", HB_UNKNOWN_STRING_SIZE, _name, HB_UNKNOWN_STRING_SIZE );
    hb_db_make_int32_value( new_values, "delay", HB_UNKNOWN_STRING_SIZE, _delay );

    hb_time_t t;
    hb_time( &t );

    hb_db_make_time_value( new_values, "start", HB_UNKNOWN_STRING_SIZE, t );

    hb_uid_t tuid;
    if( hb_db_new_document_by_name( _client, "hb_events", new_values, &tuid ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( new_values );

    *_tuid = tuid;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static uint32_t __hb_events_topic_index( const hb_events_topic_handle_t * _handle )
{
    hb_time_t t;
    hb_time( &t );

    uint64_t topic_index = (t - _handle->start) / _handle->delay;

    uint32_t index = (uint32_t)topic_index;

    return index;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_events_get_topic( hb_events_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _tuid, hb_events_topic_handle_t ** _topic )
{
    hb_events_topic_key_t key;
    key.puid = _puid;
    key.tuid = _tuid;

    hb_events_topic_handle_t * topic_handle = (hb_events_topic_handle_t *)hb_hashtable_find( _handle->ht_topics, &key, sizeof( hb_events_topic_key_t ) );

    if( topic_handle == HB_NULLPTR )
    {
        hb_db_values_handle_t * find_values;
        if( hb_db_create_values( &find_values ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_make_uid_value( find_values, "_id", HB_UNKNOWN_STRING_SIZE, _tuid );
        hb_db_make_uid_value( find_values, "puid", HB_UNKNOWN_STRING_SIZE, _puid );

        const char * fields[] = {"name", "delay", "start"};
        hb_db_values_handle_t * fields_values;

        hb_bool_t exist;
        if( hb_db_find_oid_with_values_by_name( _client, "hb_events", find_values, HB_NULLPTR, fields, sizeof( fields ) / sizeof( fields[0] ), &fields_values, &exist ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        char topic_name[32];
        if( hb_db_copy_string_value( fields_values, 0, topic_name, 32 ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        uint32_t topic_delay;
        if( hb_db_get_uint32_value( fields_values, 1, &topic_delay ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_time_t topic_start;
        if( hb_db_get_time_value( fields_values, 1, &topic_start ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_destroy_values( fields_values );
        hb_db_destroy_values( find_values );

        if( exist == HB_FALSE )
        {
            *_topic = HB_NULLPTR;

            return HB_SUCCESSFUL;
        }

        topic_handle = HB_NEW( hb_events_topic_handle_t );

        topic_handle->start = topic_start;
        topic_handle->delay = topic_delay;
        topic_handle->index = __hb_events_topic_index( topic_handle );
        memcpy( topic_handle->name, topic_name, 32 );
        

        hb_mutex_handle_t * mutex;
        if( hb_mutex_create( &mutex ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        topic_handle->mutex = mutex;

        if( hb_hashtable_emplace( _handle->ht_topics, &key, sizeof( hb_events_topic_key_t ), topic_handle ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }

    *_topic = topic_handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_events_get_topic( hb_events_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _tuid, const char ** _message, hb_error_code_t * _code )
{
    hb_events_topic_handle_t * topic_handle;
    if( __hb_events_get_topic( _handle, _client, _puid, _tuid, &topic_handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( topic_handle == HB_NULLPTR )
    {
        *_code = HB_ERROR_NOT_FOUND;

        return HB_SUCCESSFUL;
    }

    *_code = HB_ERROR_OK;

    uint32_t index = __hb_events_topic_index( topic_handle );

    if( topic_handle->index == index )
    {
        *_message = topic_handle->message;

        return HB_SUCCESSFUL;
    }

    //ToDo;

    topic_handle->index = index;

    *_message = topic_handle->message;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////