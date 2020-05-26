#include "hb_events.h"

#include "hb_mutex/hb_mutex.h"
#include "hb_memory/hb_memory.h"
#include "hb_log/hb_log.h"
#include "hb_utils/hb_hashtable.h"
#include "hb_utils/hb_list.h"
#include "hb_utils/hb_oid.h"
#include "hb_utils/hb_base16.h"

//////////////////////////////////////////////////////////////////////////
typedef struct hb_events_handle_t
{
    hb_mutex_handle_t * mutex;
    hb_hashtable_t * ht_topics;
} hb_events_handle_t;
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
hb_result_t hb_events_new_topic( hb_events_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t * _tuid )
{
    HB_UNUSED( _handle );
    HB_UNUSED( _client );
    HB_UNUSED( _puid );
    HB_UNUSED( _tuid );

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_events_get_topic( hb_events_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _tuid, hb_error_code_t * _code )
{
    HB_UNUSED( _handle );
    HB_UNUSED( _client );
    HB_UNUSED( _puid );
    HB_UNUSED( _tuid );
    HB_UNUSED( _code );

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////