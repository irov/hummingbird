#include "hb_contracts.h"

#include "hb_json/hb_json.h"
#include "hb_mutex/hb_mutex.h"
#include "hb_memory/hb_memory.h"
#include "hb_log/hb_log.h"
#include "hb_utils/hb_hashtable.h"
#include "hb_utils/hb_list.h"
#include "hb_utils/hb_base16.h"

#include "string.h"
#include "stdio.h"

//////////////////////////////////////////////////////////////////////////
typedef struct hb_contracts_handle_t
{
    hb_mutex_handle_t * mutex;
    hb_hashtable_t * ht_contracts;
} hb_contracts_handle_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_contracts_create( hb_contracts_handle_t ** _handle )
{
    hb_contracts_handle_t * handle = HB_NEW( hb_contracts_handle_t );

    hb_hashtable_t * ht_contracts;
    if( hb_hashtable_create( 1024, &ht_contracts ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    handle->ht_contracts = ht_contracts;

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
void hb_events_destroy( hb_contracts_handle_t * _handle )
{
    hb_hashtable_destroy( _handle->ht_contracts );
    hb_mutex_destroy( _handle->mutex );

    HB_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_json_visitor( const char * _key, hb_json_handle_t * _value, void * _ud )
{
    HB_UNUSED( _key );
    HB_UNUSED( _value );
    HB_UNUSED( _ud );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_contracts_new_records( hb_contracts_handle_t * _handle, hb_uid_t _puid, const void * _data, size_t _datasize )
{
    HB_UNUSED( _puid );

    hb_json_handle_t * json_data;
    if( hb_json_create( _data, _datasize, &json_data ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_foreach( json_data, &__hb_json_visitor, _handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_destroy( json_data );

    return HB_SUCCESSFUL;
}