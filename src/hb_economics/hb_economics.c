#include "hb_economics.h"

#include "hb_json/hb_json.h"
#include "hb_mutex/hb_mutex.h"
#include "hb_memory/hb_memory.h"
#include "hb_log/hb_log.h"
#include "hb_utils/hb_hashtable.h"
#include "hb_utils/hb_array.h"
#include "hb_utils/hb_base16.h"

#include "string.h"
#include "stdio.h"

//////////////////////////////////////////////////////////////////////////
typedef struct hb_economics_handle_t
{
    hb_mutex_handle_t * mutex;
    hb_hashtable_t * ht_contracts;
} hb_economics_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_economics_record_handle_t
{
    char name[32];
    uint32_t time;
    hb_json_handle_t * tags;
    hb_json_handle_t * conditions;
    hb_json_handle_t * pay;
    hb_json_handle_t * reward;
} hb_economics_record_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_economics_records_handle_t
{
    hb_array_t * array_records;
} hb_economics_records_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_economics_records_vocabulary_handle_t
{
    hb_mutex_handle_t * mutex;
    hb_hashtable_t * ht_records;
} hb_economics_records_vocabulary_handle_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_economics_create( hb_economics_handle_t ** _handle )
{
    hb_economics_handle_t * handle = HB_NEW( hb_economics_handle_t );

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
void hb_economics_destroy( hb_economics_handle_t * _handle )
{
    hb_hashtable_destroy( _handle->ht_contracts );
    hb_mutex_destroy( _handle->mutex );

    HB_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_economics_new_bank( hb_economics_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _uuid, const void * _data, size_t _datasize, hb_uid_t * _uid )
{
    HB_UNUSED( _handle );

    hb_db_values_handle_t * new_values;
    if( hb_db_create_values( &new_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_uid_value( new_values, "uuid", HB_UNKNOWN_STRING_SIZE, _uuid );
    hb_db_make_binary_value( new_values, "data", HB_UNKNOWN_STRING_SIZE, _data, _datasize );

    hb_uid_t buid;
    if( hb_db_new_document_by_name( _client, _puid, "banks", new_values, &buid ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( new_values );

    *_uid = buid;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_economics_new_records( hb_economics_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, const void * _data, size_t _datasize )
{
    HB_UNUSED( _handle );

    hb_db_collection_handle_t * db_collection_projects;
    if( hb_db_get_collection( _client, "hb", "projects", &db_collection_projects ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_values_handle_t * update_values;
    if( hb_db_create_values( &update_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_binary_value( update_values, "records", HB_UNKNOWN_STRING_SIZE, _data, _datasize );

    if( hb_db_update_values( db_collection_projects, _puid, update_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( update_values );

    hb_db_destroy_collection( db_collection_projects );

    return HB_SUCCESSFUL;
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
static hb_result_t __hb_economics_get_records( hb_economics_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_economics_records_vocabulary_handle_t ** _records )
{
    hb_economics_records_vocabulary_handle_t * records_handle = (hb_economics_records_vocabulary_handle_t *)hb_hashtable_find( _handle->ht_contracts, &_puid, sizeof( hb_uid_t ) );

    if( records_handle != HB_NULLPTR )
    {
        *_records = records_handle;

        return HB_SUCCESSFUL;
    }

    hb_db_collection_handle_t * db_collection_projects;
    if( hb_db_get_collection( _client, "hb", "projects", &db_collection_projects ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    const char * fields[] = {"records"};
    hb_db_values_handle_t * fields_values;

    hb_bool_t exist;
    if( hb_db_get_values( db_collection_projects, _puid, fields, sizeof( fields ) / sizeof( fields[0] ), &fields_values, &exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( exist == HB_FALSE )
    {
        hb_db_destroy_collection( db_collection_projects );

        *_records = HB_NULLPTR;

        return HB_SUCCESSFUL;
    }

    const void * data;
    size_t datasize;
    if( hb_db_get_binary_value( fields_values, 0, &data, &datasize ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_handle_t * json_data;
    if( hb_json_create( data, datasize, &json_data ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( fields_values );
    hb_db_destroy_collection( db_collection_projects );

    records_handle = HB_NEW( hb_economics_records_vocabulary_handle_t );

    hb_mutex_handle_t * mutex;
    if( hb_mutex_create( &mutex ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    records_handle->mutex = mutex;

    if( hb_json_foreach( json_data, &__hb_json_visitor, _handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_destroy( json_data );

    if( hb_hashtable_emplace( _handle->ht_contracts, &_puid, sizeof( hb_uid_t ), records_handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    *_records = records_handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_economics_new_contract( hb_economics_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _uuid, hb_uid_t _buid, const char * _category, const char * _name )
{
    HB_UNUSED( _uuid );
    HB_UNUSED( _buid );
    HB_UNUSED( _category );
    HB_UNUSED( _name );

    hb_economics_records_vocabulary_handle_t * records_handle;
    if( __hb_economics_get_records( _handle, _client, _puid, &records_handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}