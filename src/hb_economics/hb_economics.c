#include "hb_economics.h"

#include "hb_json/hb_json.h"
#include "hb_mutex/hb_mutex.h"
#include "hb_memory/hb_memory.h"
#include "hb_log/hb_log.h"
#include "hb_utils/hb_hashtable.h"
#include "hb_utils/hb_vectorptr.h"
#include "hb_utils/hb_base16.h"

#include "string.h"
#include "stdio.h"

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
    hb_vectorptr_t * vector_records;
} hb_economics_records_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_economics_records_vocabulary_handle_t
{
    hb_mutex_handle_t * mutex;
    hb_hashtable_t * ht_records;
} hb_economics_records_vocabulary_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_economics_bank_handle_t
{
    hb_json_handle_t * goods;
} hb_economics_bank_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_economics_banks_vocabulary_handle_t
{
    hb_mutex_handle_t * mutex;
    hb_hashtable_t * ht_banks;
} hb_economics_banks_vocabulary_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_economics_handle_t
{
    hb_mutex_handle_t * mutex;
    hb_hashtable_t * ht_contracts;
    hb_hashtable_t * ht_banks;
} hb_economics_handle_t;
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
hb_result_t hb_economics_new_bank( hb_economics_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _uuid, const void * _data, hb_size_t _datasize, hb_uid_t * _buid )
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

    *_buid = buid;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_economics_new_records( hb_economics_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, const void * _data, hb_size_t _datasize )
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
static hb_result_t __hb_json_vocabulary( const char * _key, const hb_json_handle_t * _value, void * _ud )
{
    hb_economics_records_vocabulary_handle_t * handle = (hb_economics_records_vocabulary_handle_t *)_ud;

    if( hb_json_is_array( _value ) == HB_FALSE )
    {
        return HB_FAILURE;
    }

    hb_economics_records_handle_t * records = HB_NEW( hb_economics_records_handle_t );

    uint32_t records_count = hb_json_array_count( _value );

    hb_vectorptr_create( records_count, &records->vector_records );

    for( uint32_t record_index = 0; record_index != records_count; ++record_index )
    {
        hb_json_handle_t * jrecord;
        if( hb_json_array_get( _value, record_index, &jrecord ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_economics_record_handle_t * record = HB_NEW( hb_economics_record_handle_t );

        hb_bool_t required_successful = HB_TRUE;
        if( hb_json_copy_field_string_required( jrecord, "name", record->name, 32, &required_successful ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        if( required_successful == HB_FALSE )
        {
            return HB_FAILURE;
        }

        if( hb_json_get_field_uint32( jrecord, "time", &record->time, 0U ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        if( hb_json_get_field_required( jrecord, "tags", &record->tags, HB_NULLPTR ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        if( hb_json_get_field_required( jrecord, "conditions", &record->conditions, HB_NULLPTR ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        if( hb_json_get_field_required( jrecord, "pay", &record->pay, HB_NULLPTR ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        if( hb_json_get_field_required( jrecord, "reward", &record->reward, HB_NULLPTR ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        if( hb_vectorptr_set( records->vector_records, record_index, record ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }

    if( hb_hashtable_emplace( handle->ht_records, _key, HB_UNKNOWN_STRING_SIZE, records ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_economics_cache_records_vocabulary( hb_economics_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_economics_records_vocabulary_handle_t ** _records )
{
    hb_economics_records_vocabulary_handle_t * found_records_handle = (hb_economics_records_vocabulary_handle_t *)hb_hashtable_find( _handle->ht_contracts, &_puid, sizeof( hb_uid_t ) );

    if( found_records_handle != HB_NULLPTR )
    {
        *_records = found_records_handle;

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
    hb_size_t datasize;
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

    hb_economics_records_vocabulary_handle_t * new_records_handle = HB_NEW( hb_economics_records_vocabulary_handle_t );

    hb_mutex_handle_t * mutex;
    if( hb_mutex_create( &mutex ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    new_records_handle->mutex = mutex;

    if( hb_json_object_foreach( json_data, &__hb_json_vocabulary, new_records_handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_destroy( json_data );

    if( hb_hashtable_emplace( _handle->ht_contracts, &_puid, sizeof( hb_uid_t ), new_records_handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    *_records = new_records_handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_bool_t __find_economics_record( void * _ptr, const void * _ud )
{
    hb_economics_record_handle_t * record = (hb_economics_record_handle_t *)_ptr;

    const char * name = (const char *)_ud;

    if( strcmp( record->name, name ) != 0 )
    {
        return HB_FALSE;
    }

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_economics_cache_banks_vocabulary( hb_economics_handle_t * _handle, hb_uid_t _puid, hb_economics_banks_vocabulary_handle_t ** _banks )
{
    hb_economics_banks_vocabulary_handle_t * found_banks_handle = (hb_economics_banks_vocabulary_handle_t *)hb_hashtable_find( _handle->ht_contracts, &_puid, sizeof( hb_uid_t ) );

    if( found_banks_handle != HB_NULLPTR )
    {
        *_banks = found_banks_handle;

        return HB_SUCCESSFUL;
    }

    hb_economics_banks_vocabulary_handle_t * new_banks_handle = HB_NEW( hb_economics_banks_vocabulary_handle_t );

    hb_mutex_handle_t * mutex;
    if( hb_mutex_create( &mutex ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    new_banks_handle->mutex = mutex;

    if( hb_hashtable_emplace( _handle->ht_banks, &_puid, sizeof( hb_uid_t ), new_banks_handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    *_banks = new_banks_handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_economics_cache_bank( hb_economics_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _buid, hb_economics_bank_handle_t ** _bank )
{
    hb_economics_banks_vocabulary_handle_t * vocabulary_handle;
    if( __hb_economics_cache_banks_vocabulary( _handle, _puid, &vocabulary_handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_economics_bank_handle_t * found_bank_handle = (hb_economics_bank_handle_t *)hb_hashtable_find( vocabulary_handle->ht_banks, &_buid, sizeof( hb_uid_t ) );

    if( found_bank_handle != HB_NULLPTR )
    {
        *_bank = found_bank_handle;

        return HB_SUCCESSFUL;
    }

    hb_db_values_handle_t * find_values;
    if( hb_db_create_values( &find_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_uid_value( find_values, "_id", HB_UNKNOWN_STRING_SIZE, _buid );

    const char * fields[] = {"data"};
    hb_db_values_handle_t * fields_values;

    hb_bool_t exist;
    if( hb_db_find_uid_with_values_by_name( _client, _puid, "banks", find_values, HB_NULLPTR, fields, sizeof( fields ) / sizeof( fields[0] ), &fields_values, &exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    //ToDo

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_economics_new_contract( hb_economics_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _uuid, hb_uid_t _buid, const char * _category, const char * _name, hb_error_code_t * _code )
{
    HB_UNUSED( _uuid );
    HB_UNUSED( _buid );
    HB_UNUSED( _category );
    HB_UNUSED( _name );

    hb_economics_records_vocabulary_handle_t * vocabulary_handle;
    if( __hb_economics_cache_records_vocabulary( _handle, _client, _puid, &vocabulary_handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_mutex_lock( vocabulary_handle->mutex );

    hb_economics_records_handle_t * records_handle = hb_hashtable_find( vocabulary_handle->ht_records, _category, HB_UNKNOWN_STRING_SIZE );

    if( records_handle == HB_NULLPTR )
    {
        hb_mutex_unlock( vocabulary_handle->mutex );

        *_code = HB_ERROR_NOT_FOUND;

        return HB_SUCCESSFUL;
    }

    hb_economics_record_handle_t * record_handle = hb_vectorptr_find( records_handle->vector_records, &__find_economics_record, _name );

    if( record_handle == HB_NULLPTR )
    {
        hb_mutex_unlock( vocabulary_handle->mutex );

        *_code = HB_ERROR_NOT_FOUND;

        return HB_SUCCESSFUL;
    }

    hb_mutex_unlock( vocabulary_handle->mutex );

    return HB_SUCCESSFUL;
}