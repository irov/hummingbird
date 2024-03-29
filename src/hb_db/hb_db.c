#include "hb_db.h"

#include "hb_memory/hb_memory.h"
#include "hb_log/hb_log.h"
#include "hb_utils/hb_sha1.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_vector.h"

#include "mongoc/mongoc.h"

//////////////////////////////////////////////////////////////////////////
#ifndef HB_DB_VALUE_HANDLE_MAX_VALUES
#define HB_DB_VALUE_HANDLE_MAX_VALUES 16
#endif
//////////////////////////////////////////////////////////////////////////
typedef enum hb_db_value_mode_e
{
    e_hb_db_mode_set,
    e_hb_db_mode_inc,
} hb_db_value_mode_e;
//////////////////////////////////////////////////////////////////////////
typedef enum hb_db_value_type_e
{
    e_hb_db_int32,
    e_hb_db_int64,
    e_hb_db_symbol,
    e_hb_db_utf8,
    e_hb_db_binary,
    e_hb_db_time,
    e_hb_db_dictionary,
} hb_db_value_type_e;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_db_value_handle_t
{
    hb_db_value_mode_e mode;
    hb_db_value_type_e type;

    const char * field;
    hb_size_t field_length;

    union
    {
        struct
        {
            const char * buffer;
            hb_size_t length;
        } symbol;

        struct
        {
            const char * buffer;
            hb_size_t length;
        } utf8;

        struct
        {
            const void * buffer;
            hb_size_t length;
        } binary;

        int32_t i32;
        int64_t i64;
        hb_time_t time;

        struct hb_db_values_handle_t * dict;
    } u;
} hb_db_value_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_db_values_handle_t
{
    mongoc_cursor_t * cursor;

    hb_db_value_handle_t values[16];
    uint32_t value_count;
} hb_db_values_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_db_client_handle_t
{
    mongoc_client_t * mongo_client;
} hb_db_client_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_db_collection_handle_t
{
    mongoc_collection_t * mongo_collection;
} hb_db_collection_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_db_handle_t
{
    mongoc_client_pool_t * mongo_pool;
} hb_db_handle_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_initialze( const char * _uri, const char * _host, uint16_t _port, hb_db_handle_t ** _handle )
{
    hb_db_handle_t * handle = HB_NEW( hb_db_handle_t );

    HB_LOG_MESSAGE_INFO( "db", "mongo try..." );

    mongoc_init();

    HB_LOG_MESSAGE_INFO( "db", "mongo init" );

    mongoc_uri_t * mongoc_uri = HB_NULLPTR;

    if( strlen( _uri ) != 0 )
    {    
        bson_error_t error;
        mongoc_uri = mongoc_uri_new_with_error( _uri, &error );

        if( mongoc_uri == HB_NULLPTR )
       {
            HB_LOG_MESSAGE_ERROR( "db", "failed to use uri: %s error domain: %u code: %u message: %s"
                , _uri
                , error.domain
                , error.code
                , error.message
            );

            return HB_FAILURE;
        }
    }
    else
    {
        mongoc_uri = mongoc_uri_new_for_host_port( _host, _port );

        if( mongoc_uri == HB_NULLPTR )
        {
            HB_LOG_MESSAGE_ERROR( "db", "failed to use host: %s port: %" PRIu16
                , _host
                , _port
            );

            return HB_FAILURE;
        }
    }

    mongoc_client_pool_t * mongo_pool = mongoc_client_pool_new( mongoc_uri );

    mongoc_uri_destroy( mongoc_uri );

    if( mongoc_client_pool_set_error_api( mongo_pool, MONGOC_ERROR_API_VERSION_2 ) == false )
    {
        HB_LOG_MESSAGE_ERROR( "db", "failed to set error api: %s:%" PRIu16
            , _host
            , _port
        );

        return HB_FAILURE;
    }

    HB_LOG_MESSAGE_INFO( "db", "ping try..." );

    bson_t ping;
    bson_init( &ping );

    bson_append_int32( &ping, "ping", sizeof( "ping" ) - 1, 1 );

    mongoc_client_t * mongo_client = mongoc_client_pool_pop( mongo_pool );

    bson_error_t error;
    bool mongoc_ping = mongoc_client_command_simple( mongo_client, "admin", &ping, NULL, NULL, &error );

    mongoc_client_pool_push( mongo_pool, mongo_client );

    bson_destroy( &ping );

    if( mongoc_ping == false )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid ping error: %s code: %u"
            , error.message
            , error.code
        );

        return HB_FAILURE;
    }

    HB_LOG_MESSAGE_INFO( "db", "ping successful" );

    HB_LOG_MESSAGE_INFO( "db", "create pool url:'%s' port:%" PRIu16 " successful"
        , _host
        , _port
    );

    handle->mongo_pool = mongo_pool;

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_finalize( hb_db_handle_t * _handle )
{
    mongoc_client_pool_destroy( _handle->mongo_pool );
    _handle->mongo_pool = HB_NULLPTR;

    mongoc_cleanup();
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_create_client( hb_db_handle_t * _handle, hb_db_client_handle_t ** _client )
{
    hb_db_client_handle_t * handle = HB_NEW( hb_db_client_handle_t );

    mongoc_client_t * mongo_client = mongoc_client_pool_pop( _handle->mongo_pool );
    handle->mongo_client = mongo_client;

    *_client = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_destroy_client( hb_db_handle_t * _handle, hb_db_client_handle_t * _client )
{
    mongoc_client_t * mongo_client = _client->mongo_client;
    mongoc_client_pool_push( _handle->mongo_pool, mongo_client );

    HB_DELETE( _client );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_collection( const hb_db_client_handle_t * _client, const char * _db, const char * _name, hb_db_collection_handle_t ** _handle )
{
    mongoc_client_t * mongo_client = _client->mongo_client;

    mongoc_collection_t * collection = mongoc_client_get_collection( mongo_client, _db, _name );

    hb_db_collection_handle_t * handle = HB_NEW( hb_db_collection_handle_t );

    handle->mongo_collection = collection;

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_project_collection( const hb_db_client_handle_t * _client, hb_uid_t _uid, const char * _name, hb_db_collection_handle_t ** _collection )
{
    char db_uid[64];
    sprintf( db_uid, "hb_%d", _uid );

    hb_result_t result = hb_db_get_collection( _client, db_uid, _name, _collection );

    return result;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_destroy_collection( hb_db_collection_handle_t * _handle )
{
    mongoc_collection_t * mongo_collection = _handle->mongo_collection;
    mongoc_collection_destroy( mongo_collection );

    HB_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_set_collection_expire( const hb_db_collection_handle_t * _handle, const char * _field, uint32_t _expire )
{
    mongoc_collection_t * mongo_collection = _handle->mongo_collection;

    mongoc_index_opt_t opt;
    mongoc_index_opt_init( &opt );

    opt.expire_after_seconds = _expire;

    bson_t keys;
    bson_init( &keys );
    bson_append_int64( &keys, _field, strlen( _field ), 1 );

    bson_error_t error;
    if( mongoc_collection_create_index_with_opts( mongo_collection, &keys, &opt, HB_NULLPTR, HB_NULLPTR, &error ) == false )
    {
        HB_LOG_MESSAGE_ERROR( "db", "failed to set collection expire: %s\nerror message: %s\n"
            , mongoc_collection_get_name( mongo_collection )
            , error.message
        );

        return HB_FAILURE;
    }

    bson_destroy( &keys );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_db_append_value( bson_t * _bson, const hb_db_value_handle_t * _handle )
{
    switch( _handle->type )
    {
    case e_hb_db_int32:
        {
            bson_append_int32( _bson, _handle->field, _handle->field_length, _handle->u.i32 );
        }break;
    case e_hb_db_int64:
        {
            bson_append_int64( _bson, _handle->field, _handle->field_length, _handle->u.i64 );
        }break;
    case e_hb_db_symbol:
        {
            bson_append_symbol( _bson, _handle->field, _handle->field_length, _handle->u.symbol.buffer, _handle->u.symbol.length );
        }break;
    case e_hb_db_binary:
        {
            bson_append_binary( _bson, _handle->field, _handle->field_length, BSON_SUBTYPE_BINARY, _handle->u.binary.buffer, _handle->u.binary.length );
        }break;
    case e_hb_db_time:
        {
            bson_append_time_t( _bson, _handle->field, _handle->field_length, (time_t)_handle->u.time );
        }break;
    case e_hb_db_dictionary:
        {
            bson_t dict;
            if( bson_append_document_begin( _bson, _handle->field, _handle->field_length, &dict ) == false )
            {
                return HB_FAILURE;
            }

            hb_size_t dict_count = _handle->u.dict->value_count;

            for( hb_size_t dict_index = 0; dict_index != dict_count; ++dict_index )
            {
                hb_db_value_handle_t * dict_handle = _handle->u.dict->values + dict_index;

                __hb_db_append_value( &dict, dict_handle );
            }

            if( bson_append_document_end( _bson, &dict ) == false )
            {
                return HB_FAILURE;
            }
        }
    default:
        {
            return HB_FAILURE;
        }break;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_bool_t __hb_db_has_values_mode( const hb_db_values_handle_t * _handles, hb_db_value_mode_e _mode )
{
    uint32_t value_count = _handles->value_count;

    for( uint32_t index = 0; index != value_count; ++index )
    {
        const hb_db_value_handle_t * handle = _handles->values + index;

        if( handle->mode != _mode )
        {
            continue;
        }

        return HB_TRUE;
    }

    return HB_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_db_append_values( bson_t * _bson, const hb_db_values_handle_t * _handles, hb_db_value_mode_e _mode )
{
    uint32_t value_count = _handles->value_count;

    for( uint32_t index = 0; index != value_count; ++index )
    {
        const hb_db_value_handle_t * handle = _handles->values + index;

        if( handle->mode != _mode )
        {
            continue;
        }

        if( __hb_db_append_value( _bson, handle ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_new_document( const hb_db_collection_handle_t * _collection, const hb_db_values_handle_t * _values, hb_uid_t * _newuid )
{
    mongoc_collection_t * mongo_collection = _collection->mongo_collection;

    hb_uid_t uid = 0;
    for( ;; )
    {
        do
        {
            uid = hb_rand_time();
            uid &= 0x7fffffff;
        } while( uid == 0 );

        bson_t query;
        bson_init( &query );

        BSON_APPEND_INT32( &query, "_id", uid );

        if( __hb_db_append_values( &query, _values, e_hb_db_mode_set ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        bson_error_t error;
        bool successful = mongoc_collection_insert_one( mongo_collection, &query, HB_NULLPTR, HB_NULLPTR, &error );

        bson_destroy( &query );

        if( successful == false )
        {
            if( error.code == 11000 )
            {
                continue;
            }

            HB_LOG_MESSAGE_ERROR( "db", "invalid new document: %s"
                , error.message
            );

            return HB_FAILURE;
        }

        break;
    }

    *_newuid = uid;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_new_document_by_name( const hb_db_client_handle_t * _client, uint32_t _puid, const char * _name, const hb_db_values_handle_t * _values, hb_uid_t * _newuid )
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_project_collection( _client, _puid, _name, &db_collection ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid get collection '%s'"
            , _name
        );

        return HB_FAILURE;
    }

    hb_result_t result = hb_db_new_document( db_collection, _values, _newuid );

    hb_db_destroy_collection( db_collection );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_create_values( hb_db_values_handle_t ** _values )
{
    hb_db_values_handle_t * values = HB_NEW( hb_db_values_handle_t );

    values->cursor = HB_NULLPTR;
    values->value_count = 0;

    *_values = values;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_destroy_values( hb_db_values_handle_t * _values )
{
    if( _values->cursor != HB_NULLPTR )
    {
        mongoc_cursor_destroy( _values->cursor );
    }

    HB_DELETE( _values );
}
//////////////////////////////////////////////////////////////////////////
void hb_db_copy_values( hb_db_values_handle_t * _values, const hb_db_values_handle_t * _source )
{
    if( _source == HB_NULLPTR )
    {
        return;
    }

    for( hb_size_t index = 0; index != _source->value_count; ++index )
    {
        _values->values[index] = _source->values[index];
    }

    _values->value_count += _source->value_count;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_uid_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, hb_uid_t _value )
{
    hb_db_value_handle_t * value = _values->values + _values->value_count;
    ++_values->value_count;

    value->mode = e_hb_db_mode_set;
    value->type = e_hb_db_int32;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.i32 = (int32_t)_value;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_int32_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, int32_t _value )
{
    hb_db_value_handle_t * value = _values->values + _values->value_count;
    ++_values->value_count;

    value->mode = e_hb_db_mode_set;
    value->type = e_hb_db_int32;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.i32 = _value;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_int64_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, int64_t _value )
{
    hb_db_value_handle_t * value = _values->values + _values->value_count;
    ++_values->value_count;

    value->mode = e_hb_db_mode_set;
    value->type = e_hb_db_int64;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.i64 = _value;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_string_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, const char * _string, hb_size_t _stringlength )
{
    hb_db_value_handle_t * value = _values->values + _values->value_count;
    ++_values->value_count;

    value->mode = e_hb_db_mode_set;
    value->type = e_hb_db_symbol;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.symbol.buffer = _string;
    value->u.symbol.length = _stringlength == HB_UNKNOWN_STRING_SIZE ? strlen( _string ) : _stringlength;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_binary_value( hb_db_values_handle_t * _handles, const char * _field, hb_size_t _fieldlength, const void * _buffer, hb_size_t _bufferlength )
{
    hb_db_value_handle_t * value = _handles->values + _handles->value_count;
    ++_handles->value_count;

    value->mode = e_hb_db_mode_set;
    value->type = e_hb_db_binary;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.binary.buffer = _buffer;
    value->u.binary.length = _bufferlength;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_time_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, hb_time_t _time )
{
    hb_db_value_handle_t * value = _values->values + _values->value_count;
    ++_values->value_count;

    value->mode = e_hb_db_mode_set;
    value->type = e_hb_db_time;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.time = _time;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_sha1_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, const hb_sha1_t * _sha1 )
{
    hb_db_value_handle_t * value = _values->values + _values->value_count;
    ++_values->value_count;

    value->mode = e_hb_db_mode_set;
    value->type = e_hb_db_binary;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.binary.buffer = _sha1->value;
    value->u.binary.length = sizeof( hb_sha1_t );
}
//////////////////////////////////////////////////////////////////////////
void hb_db_inc_int32_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, int32_t _value )
{
    hb_db_value_handle_t * value = _values->values + _values->value_count;
    ++_values->value_count;

    value->mode = e_hb_db_mode_inc;
    value->type = e_hb_db_int32;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.i32 = _value;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_make_dictionary_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, hb_db_values_handle_t ** _dictionary )
{
    hb_db_value_handle_t * value = _values->values + _values->value_count;
    ++_values->value_count;

    value->mode = e_hb_db_mode_set;
    value->type = e_hb_db_dictionary;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;

    if( hb_db_create_values( &value->u.dict ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    *_dictionary = value->u.dict;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_uid_value( const hb_db_values_handle_t * _values, uint32_t _index, hb_uid_t * _value )
{
    if( _index >= _values->value_count )
    {
        return HB_FAILURE;
    }

    const hb_db_value_handle_t * value = _values->values + _index;

    if( value->type != e_hb_db_int32 )
    {
        return HB_FAILURE;
    }

    *_value = (hb_uid_t)value->u.i32;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_int32_value( const hb_db_values_handle_t * _values, uint32_t _index, int32_t * _value )
{
    if( _index >= _values->value_count )
    {
        return HB_FAILURE;
    }

    const hb_db_value_handle_t * value = _values->values + _index;

    if( value->type != e_hb_db_int32 )
    {
        return HB_FAILURE;
    }

    *_value = value->u.i32;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_uint32_value( const hb_db_values_handle_t * _values, uint32_t _index, uint32_t * _value )
{
    if( _index >= _values->value_count )
    {
        return HB_FAILURE;
    }

    const hb_db_value_handle_t * value = _values->values + _index;

    if( value->type != e_hb_db_int32 )
    {
        return HB_FAILURE;
    }

    *_value = (uint32_t)value->u.i32;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_int64_value( const hb_db_values_handle_t * _values, uint32_t _index, int64_t * _value )
{
    if( _index >= _values->value_count )
    {
        return HB_FAILURE;
    }

    const hb_db_value_handle_t * value = _values->values + _index;

    if( value->type != e_hb_db_int64 )
    {
        return HB_FAILURE;
    }

    *_value = value->u.i64;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_string_value( const hb_db_values_handle_t * _values, uint32_t _index, const char ** _value, hb_size_t * _length )
{
    if( _index >= _values->value_count )
    {
        return HB_FAILURE;
    }

    const hb_db_value_handle_t * value = _values->values + _index;

    if( value->type != e_hb_db_symbol )
    {
        return HB_FAILURE;
    }

    *_value = value->u.symbol.buffer;
    *_length = value->u.symbol.length;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_json_value( const hb_db_values_handle_t * _values, uint32_t _index, void * _pool, hb_size_t _capacity, const hb_json_handle_t ** _value )
{
    if( _index >= _values->value_count )
    {
        return HB_FAILURE;
    }

    const hb_db_value_handle_t * value = _values->values + _index;

    if( value->type != e_hb_db_symbol )
    {
        return HB_FAILURE;
    }

    hb_json_handle_t * json_value;
    if( hb_json_create( value->u.symbol.buffer, value->u.symbol.length, _pool, _capacity, &json_value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    *_value = json_value;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_copy_string_value( const hb_db_values_handle_t * _values, uint32_t _index, char * _value, hb_size_t _capacity )
{
    if( _index >= _values->value_count )
    {
        return HB_FAILURE;
    }

    const hb_db_value_handle_t * value = _values->values + _index;

    if( value->type != e_hb_db_symbol )
    {
        return HB_FAILURE;
    }

    if( value->u.symbol.length >= _capacity )
    {
        return HB_FAILURE;
    }

    memcpy( _value, value->u.symbol.buffer, value->u.symbol.length );
    _value[value->u.symbol.length] = '\0';

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_binary_value( const hb_db_values_handle_t * _values, uint32_t _index, const void ** _buffer, hb_size_t * _length )
{
    if( _index >= _values->value_count )
    {
        return HB_FAILURE;
    }

    const hb_db_value_handle_t * value = _values->values + _index;

    if( value->type != e_hb_db_binary )
    {
        return HB_FAILURE;
    }

    *_buffer = value->u.binary.buffer;
    *_length = value->u.binary.length;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_copy_binary_value( const hb_db_values_handle_t * _values, uint32_t _index, void * _buffer, hb_size_t _capacity )
{
    if( _index >= _values->value_count )
    {
        return HB_FAILURE;
    }

    const hb_db_value_handle_t * value = _values->values + _index;

    if( value->type != e_hb_db_binary )
    {
        return HB_FAILURE;
    }

    if( value->u.binary.length != _capacity )
    {
        return HB_FAILURE;
    }

    memcpy( _buffer, value->u.binary.buffer, value->u.binary.length );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_time_value( const hb_db_values_handle_t * _values, uint32_t _index, hb_time_t * _value )
{
    if( _index >= _values->value_count )
    {
        return HB_FAILURE;
    }

    const hb_db_value_handle_t * value = _values->values + _index;

    if( value->type != e_hb_db_time )
    {
        return HB_FAILURE;
    }

    *_value = value->u.time;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_db_find_iter( const bson_t * _data, bson_iter_t * _iter, const char * _key )
{
    if( bson_iter_find( _iter, _key ) == false )
    {
        if( bson_iter_init( _iter, _data ) == false )
        {
            return HB_FAILURE;
        }

        if( bson_iter_find( _iter, _key ) == false )
        {
            return HB_FAILURE;
        }
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_exist_uid( const hb_db_collection_handle_t * _collection, hb_uid_t _uid, hb_bool_t * _exist )
{
    mongoc_collection_t * mongo_collection = _collection->mongo_collection;

    bson_t query;
    bson_init( &query );

    bson_append_int32( &query, "_id", -1, _uid );

    mongoc_cursor_t * cursor = mongoc_collection_find( mongo_collection, MONGOC_QUERY_NONE, 0, 1, 0, &query, HB_NULLPTR, HB_NULLPTR );

    bson_destroy( &query );

    const bson_t * data;
    bool successful = mongoc_cursor_next( cursor, &data );

    mongoc_cursor_destroy( cursor );

    if( successful == true )
    {
        *_exist = HB_TRUE;
    }
    else
    {
        *_exist = HB_FALSE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_exist_project_uid( const hb_db_client_handle_t * _client, hb_uid_t _uid, hb_bool_t * _exist )
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_collection( _client, "hb", "projects", &db_collection ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid get collection '%s'"
            , "projects"
        );

        return HB_FAILURE;
    }

    hb_result_t result = hb_db_exist_uid( db_collection, _uid, _exist );

    hb_db_destroy_collection( db_collection );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_exist_uid_by_name( const hb_db_client_handle_t * _client, uint32_t _puid, const char * _name, hb_uid_t _uid, hb_bool_t * _exist )
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_project_collection( _client, _puid, _name, &db_collection ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid get collection '%s'"
            , _name
        );

        return HB_FAILURE;
    }

    hb_result_t result = hb_db_exist_uid( db_collection, _uid, _exist );

    hb_db_destroy_collection( db_collection );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_find_uid( const hb_db_collection_handle_t * _handle, const hb_db_values_handle_t * _query, hb_uid_t * _uid, hb_bool_t * _exist )
{
    mongoc_collection_t * mongo_collection = _handle->mongo_collection;

    bson_t query;
    bson_init( &query );

    if( __hb_db_append_values( &query, _query, e_hb_db_mode_set ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    bson_t fields;
    bson_init( &fields );

    BSON_APPEND_INT32( &fields, "_id", 1 );

    mongoc_cursor_t * cursor = mongoc_collection_find( mongo_collection, MONGOC_QUERY_NONE, 0, 1, 0, &query, &fields, HB_NULLPTR );

    bson_destroy( &query );
    bson_destroy( &fields );

    const bson_t * data;
    if( mongoc_cursor_next( cursor, &data ) == false )
    {
        mongoc_cursor_destroy( cursor );

        *_exist = HB_FALSE;

        return HB_SUCCESSFUL;
    }

    bson_iter_t iter;
    if( bson_iter_init( &iter, data ) == false )
    {
        mongoc_cursor_destroy( cursor );

        return HB_FAILURE;
    }

    if( __hb_db_find_iter( data, &iter, "_id" ) == HB_FAILURE )
    {
        mongoc_cursor_destroy( cursor );

        return HB_FAILURE;
    }

    int32_t uid = bson_iter_int32( &iter );

    if( _uid != HB_NULLPTR )
    {
        *_uid = uid;
    }

    mongoc_cursor_destroy( cursor );

    *_exist = HB_TRUE;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_find_uid_by_name( const hb_db_client_handle_t * _client, uint32_t _puid, const char * _name, const hb_db_values_handle_t * _query, hb_uid_t * _uid, hb_bool_t * _exist )
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_project_collection( _client, _puid, _name, &db_collection ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid get collection '%s'"
            , _name
        );

        return HB_FAILURE;
    }

    hb_result_t result = hb_db_find_uid( db_collection, _query, _uid, _exist );

    hb_db_destroy_collection( db_collection );

    return result;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_db_get_bson_value( hb_db_value_handle_t * _value, const bson_t * _data, bson_iter_t * _iter, const char * _field )
{
    _value->field = _field;
    _value->field_length = strlen( _field );

    if( __hb_db_find_iter( _data, _iter, _field ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid find iterator for '%s'"
            , _field
        );

        return HB_FAILURE;
    }

    bson_type_t type = bson_iter_type( _iter );

    _value->mode = e_hb_db_mode_set;

    switch( type )
    {
    case BSON_TYPE_INT32:
        {
            _value->type = e_hb_db_int32;

            _value->u.i32 = bson_iter_int32( _iter );
        }break;
    case BSON_TYPE_INT64:
        {
            _value->type = e_hb_db_int64;

            _value->u.i64 = bson_iter_int64( _iter );
        }break;
    case BSON_TYPE_UTF8:
        {
            _value->type = e_hb_db_utf8;

            uint32_t utf8_length;
            const char * utf8_value = bson_iter_utf8( _iter, &utf8_length );

            _value->u.utf8.length = utf8_length;
            _value->u.utf8.buffer = utf8_value;
        }break;
    case BSON_TYPE_SYMBOL:
        {
            _value->type = e_hb_db_symbol;

            uint32_t symbol_length;
            const char * symbol_value = bson_iter_symbol( _iter, &symbol_length );

            _value->u.symbol.length = symbol_length;
            _value->u.symbol.buffer = symbol_value;
        }break;
    case BSON_TYPE_BINARY:
        {
            _value->type = e_hb_db_binary;

            bson_subtype_t binary_subtype;
            uint32_t binary_length;
            const hb_byte_t * binary_buffer;
            bson_iter_binary( _iter, &binary_subtype, &binary_length, &binary_buffer );

            _value->u.binary.length = binary_length;
            _value->u.binary.buffer = binary_buffer;
        }break;
    case BSON_TYPE_DATE_TIME:
        {
            _value->type = e_hb_db_time;

            _value->u.time = bson_iter_time_t( _iter );
        }break;
    default:
        {
            HB_LOG_MESSAGE_ERROR( "db", "invalid get bson value for '%s' unknown type: %u"
                , _field
                , type
            );

            return HB_FAILURE;
        }break;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_find_uid_with_values( const hb_db_collection_handle_t * _handle, const hb_db_values_handle_t * _query, hb_uid_t * _uid, const char ** _fields, uint32_t _fieldcount, hb_db_values_handle_t ** _values, hb_bool_t * _exist )
{
    mongoc_collection_t * mongo_collection = _handle->mongo_collection;

    bson_t query;
    bson_init( &query );

    if( __hb_db_append_values( &query, _query, e_hb_db_mode_set ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    bson_t fields;
    bson_init( &fields );

    BSON_APPEND_INT32( &fields, "_id", 1 );

    for( uint32_t index = 0; index != _fieldcount; ++index )
    {
        BSON_APPEND_INT32( &fields, _fields[index], 1 );
    }

    mongoc_cursor_t * cursor = mongoc_collection_find( mongo_collection, MONGOC_QUERY_NONE, 0, 1, 0, &query, &fields, HB_NULLPTR );

    bson_destroy( &query );
    bson_destroy( &fields );

    const bson_t * data;
    if( mongoc_cursor_next( cursor, &data ) == false )
    {
        mongoc_cursor_destroy( cursor );

        *_exist = HB_FALSE;

        return HB_SUCCESSFUL;
    }

    bson_error_t error;
    if( mongoc_cursor_error( cursor, &error ) )
    {
        mongoc_cursor_destroy( cursor );

        if( error.code == 11000 )
        {
            *_exist = HB_FALSE;

            return HB_SUCCESSFUL;
        }

        HB_LOG_MESSAGE_ERROR( "db", "find with values values error occurred: %s"
            , error.message
        );

        return HB_FAILURE;
    }

    hb_db_values_handle_t * values;
    if( hb_db_create_values( &values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    values->cursor = cursor;

    bson_iter_t iter;
    if( bson_iter_init( &iter, data ) == false )
    {
        mongoc_cursor_destroy( cursor );

        return HB_FAILURE;
    }

    if( _uid != HB_NULLPTR )
    {
        if( __hb_db_find_iter( data, &iter, "_id" ) == HB_FAILURE )
        {
            mongoc_cursor_destroy( cursor );

            return HB_FAILURE;
        }

        hb_uid_t uid = (hb_uid_t)bson_iter_int32( &iter );

        *_uid = uid;
    }

    for( uint32_t index = 0; index != _fieldcount; ++index )
    {
        hb_db_value_handle_t * value = values->values + index;
        ++values->value_count;

        const char * field = _fields[index];

        if( __hb_db_get_bson_value( value, data, &iter, field ) == HB_FAILURE )
        {
            mongoc_cursor_destroy( cursor );

            return HB_FAILURE;
        }
    }

    if( _values != HB_NULLPTR )
    {
        *_values = values;
    }
    else
    {
        hb_db_destroy_values( values );
    }

    *_exist = HB_TRUE;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_find_uid_with_values_by_name( const hb_db_client_handle_t * _client, uint32_t _puid, const char * _name, const hb_db_values_handle_t * _query, hb_uid_t * _uid, const char ** _fields, uint32_t _fieldcount, hb_db_values_handle_t ** _values, hb_bool_t * _exist )
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_project_collection( _client, _puid, _name, &db_collection ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid get collection '%s'"
            , _name
        );

        return HB_FAILURE;
    }

    hb_result_t result = hb_db_find_uid_with_values( db_collection, _query, _uid, _fields, _fieldcount, _values, _exist );

    hb_db_destroy_collection( db_collection );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_select_values( const hb_db_collection_handle_t * _handle, const hb_db_values_handle_t * _query, const char ** _fields, uint32_t _fieldcount, hb_db_values_handle_t ** _values, uint32_t _limit, uint32_t * _exists )
{
    mongoc_collection_t * mongo_collection = _handle->mongo_collection;

    bson_t query;
    bson_init( &query );

    if( __hb_db_append_values( &query, _query, e_hb_db_mode_set ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    bson_t fields;
    bson_init( &fields );

    for( uint32_t index = 0; index != _fieldcount; ++index )
    {
        BSON_APPEND_INT32( &fields, _fields[index], 1 );
    }

    mongoc_cursor_t * cursor = mongoc_collection_find( mongo_collection, MONGOC_QUERY_NONE, 0, 0, 0, &query, &fields, HB_NULLPTR );

    bson_destroy( &query );
    bson_destroy( &fields );

    hb_db_values_handle_t * values;
    if( hb_db_create_values( &values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    values->cursor = cursor;

    uint32_t cursor_count = 0;

    const bson_t * data;
    while( mongoc_cursor_next( cursor, &data ) == true )
    {
        if( cursor_count == _limit )
        {
            break;
        }

        bson_iter_t iter;
        if( bson_iter_init( &iter, data ) == false )
        {
            mongoc_cursor_destroy( cursor );

            return HB_FAILURE;
        }

        for( uint32_t index = 0; index != _fieldcount; ++index )
        {
            hb_db_value_handle_t * value = values->values + index;
            ++values->value_count;

            const char * field = _fields[index];

            if( __hb_db_get_bson_value( value, data, &iter, field ) == HB_FAILURE )
            {
                mongoc_cursor_destroy( cursor );

                return HB_FAILURE;
            }
        }

        _values[cursor_count] = values;

        ++cursor_count;
    }

    bson_error_t error;
    if( mongoc_cursor_error( cursor, &error ) )
    {
        mongoc_cursor_destroy( cursor );

        HB_LOG_MESSAGE_ERROR( "db", "select values error occurred: %s"
            , error.message
        );

        return HB_FAILURE;
    }

    *_exists = cursor_count;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_count_values( const hb_db_collection_handle_t * _handle, const hb_db_values_handle_t * _query, uint32_t * _founds )
{
    mongoc_collection_t * mongo_collection = _handle->mongo_collection;

    bson_t filter;
    bson_init( &filter );

    if( __hb_db_append_values( &filter, _query, e_hb_db_mode_set ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    bson_error_t error;
    int64_t count = mongoc_collection_count_documents( mongo_collection, &filter, HB_NULLPTR, HB_NULLPTR, HB_NULLPTR, &error );

    bson_destroy( &filter );

    if( count < 0 )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid collection count documents error occurred: %s"
            , error.message
        );

        return HB_FAILURE;
    }

    *_founds = (uint32_t)count;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_gets_values( const hb_db_collection_handle_t * _collection, const hb_uid_t * _uids, uint32_t _uidcount, const char ** _fields, uint32_t _fieldscount, hb_db_values_handle_t ** _values, hb_bool_t * _exist )
{
    if( _uidcount == 0 )
    {
        hb_db_values_handle_t * values;
        if( hb_db_create_values( &values ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        values->cursor = HB_NULLPTR;

        values->value_count = 0;

        *_values = values;

        if( _exist != HB_NULLPTR )
        {
            *_exist = HB_TRUE;
        }

        return HB_SUCCESSFUL;
    }

    mongoc_collection_t * mongo_collection = _collection->mongo_collection;

    bson_t query;
    bson_init( &query );

    if( _uidcount == 1 )
    {
        hb_uid_t uid = _uids[0];

        BSON_APPEND_INT32( &query, "_id", uid );
    }
    else
    {
        bson_t query_id;
        if( bson_append_document_begin( &query, "_id", -1, &query_id ) == false )
        {
            return HB_FAILURE;
        }

        bson_t query_in;
        if( bson_append_array_begin( &query_id, "$in", -1, &query_in ) == false )
        {
            return HB_FAILURE;
        }

        for( uint32_t index = 0; index != _uidcount; ++index )
        {
            hb_uid_t uid = _uids[index];

            BSON_APPEND_INT32( &query_in, "$oid", uid );
        }

        if( bson_append_array_end( &query_id, &query_in ) == false )
        {
            return HB_FAILURE;
        }

        if( bson_append_document_end( &query, &query_id ) == false )
        {
            return HB_FAILURE;
        }
    }

    bson_t fields;
    bson_init( &fields );

    if( _uidcount == 0 )
    {
        BSON_APPEND_INT32( &fields, "_id", 1 );
    }

    for( uint32_t index = 0; index != _fieldscount; ++index )
    {
        BSON_APPEND_INT32( &fields, _fields[index], 1 );
    }

    mongoc_cursor_t * cursor = mongoc_collection_find( mongo_collection, MONGOC_QUERY_NONE, 0, 0, 0, &query, &fields, HB_NULLPTR );

    bson_error_t test_error;
    if( mongoc_cursor_error( cursor, &test_error ) )
    {
        mongoc_cursor_destroy( cursor );
        bson_destroy( &query );
        bson_destroy( &fields );

        if( test_error.code == 11000 )
        {
            if( _exist != HB_NULLPTR )
            {
                *_exist = HB_FALSE;
            }

            return HB_SUCCESSFUL;
        }

        HB_LOG_MESSAGE_ERROR( "db", "find with values values error occurred: %s"
            , test_error.message
        );

        return HB_FAILURE;
    }

    hb_db_values_handle_t * values;
    if( hb_db_create_values( &values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    values->cursor = cursor;

    for( uint32_t index_uid = 0; index_uid != _uidcount; ++index_uid )
    {
        const bson_t * data;
        if( mongoc_cursor_next( cursor, &data ) == false )
        {
            bson_error_t error;
            if( mongoc_cursor_error( cursor, &error ) )
            {
                mongoc_cursor_destroy( cursor );
                bson_destroy( &query );
                bson_destroy( &fields );

                HB_LOG_MESSAGE_ERROR( "db", "cursor next error occurred: %s [%u:%u]"
                    , error.message
                    , index_uid
                    , _uidcount
                );

                return HB_FAILURE;
            }

            mongoc_cursor_destroy( cursor );

            HB_LOG_MESSAGE_ERROR( "db", "cursor not found next [%u:%u]"
                , index_uid
                , _uidcount
            );

            return HB_FAILURE;
        }

        bson_iter_t iter;
        if( bson_iter_init( &iter, data ) == false )
        {
            mongoc_cursor_destroy( cursor );
            bson_destroy( &query );
            bson_destroy( &fields );

            HB_LOG_MESSAGE_ERROR( "db", "invalid initialize iterator [uid %u]"
                , index_uid
            );

            return HB_FAILURE;
        }

        uint32_t correct_index_uid = ~0U;

        if( _uidcount != 0 )
        {
            if( __hb_db_find_iter( data, &iter, "_id" ) == HB_FAILURE )
            {
                mongoc_cursor_destroy( cursor );
                bson_destroy( &query );
                bson_destroy( &fields );

                HB_LOG_MESSAGE_ERROR( "db", "invalid initialize iterator for '_id' [%u]"
                    , index_uid
                );

                return HB_FAILURE;
            }

            hb_uid_t uid = bson_iter_int32( &iter );

            for( uint32_t index_correct = 0; index_correct != _uidcount; ++index_correct )
            {
                hb_uid_t correct_uid = _uids[index_correct];

                if( correct_uid == uid )
                {
                    correct_index_uid = index_correct;
                }
            }

            if( correct_index_uid == ~0U )
            {
                mongoc_cursor_destroy( cursor );
                bson_destroy( &query );
                bson_destroy( &fields );

                HB_LOG_MESSAGE_ERROR( "db", "invalid find correct index [%u]"
                    , index_uid
                );

                return HB_FAILURE;
            }
        }
        else
        {
            correct_index_uid = 0;
        }

        for( uint32_t index_field = 0; index_field != _fieldscount; ++index_field )
        {
            hb_db_value_handle_t * value = values->values + correct_index_uid * _fieldscount + index_field;
            ++values->value_count;

            const char * field = _fields[index_field];

            if( __hb_db_get_bson_value( value, data, &iter, field ) == HB_FAILURE )
            {
                mongoc_cursor_destroy( cursor );
                bson_destroy( &query );
                bson_destroy( &fields );

                HB_LOG_MESSAGE_ERROR( "db", "invalid get bson value for '%s' [%u]"
                    , field
                    , index_uid
                );

                return HB_FAILURE;
            }
        }
    }

    *_values = values;

    if( _exist != HB_NULLPTR )
    {
        *_exist = HB_TRUE;
    }

    bson_destroy( &query );
    bson_destroy( &fields );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_gets_values_by_name( const hb_db_client_handle_t * _client, uint32_t _puid, const char * _name, const hb_uid_t * _uids, uint32_t _uidcount, const char ** _fields, uint32_t _fieldscount, hb_db_values_handle_t ** _values, hb_bool_t * _exist )
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_project_collection( _client, _puid, _name, &db_collection ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid get collection '%s'"
            , _name
        );

        return HB_FAILURE;
    }

    hb_result_t result = hb_db_gets_values( db_collection, _uids, _uidcount, _fields, _fieldscount, _values, _exist );

    hb_db_destroy_collection( db_collection );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_values( const hb_db_collection_handle_t * _collection, hb_uid_t _uid, const char ** _fields, uint32_t _count, hb_db_values_handle_t ** _values, hb_bool_t * _exist )
{
    hb_result_t result = hb_db_gets_values( _collection, &_uid, 1, _fields, _count, _values, _exist );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_values_by_name( const hb_db_client_handle_t * _client, uint32_t _puid, const char * _name, hb_uid_t _uid, const char ** _fields, uint32_t _count, hb_db_values_handle_t ** _values, hb_bool_t * _exist )
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_project_collection( _client, _puid, _name, &db_collection ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid get collection '%s'"
            , _name
        );

        return HB_FAILURE;
    }

    hb_result_t result = hb_db_get_values( db_collection, _uid, _fields, _count, _values, _exist );

    hb_db_destroy_collection( db_collection );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_update_values( const hb_db_collection_handle_t * _collection, hb_uid_t _uid, const hb_db_values_handle_t * _handles )
{
    mongoc_collection_t * mongo_collection = _collection->mongo_collection;

    bson_t query;
    bson_init( &query );
    BSON_APPEND_INT32( &query, "_id", _uid );

    bson_t update;
    bson_init( &update );

    if( __hb_db_has_values_mode( _handles, e_hb_db_mode_set ) == HB_TRUE )
    {
        bson_t fields;
        if( bson_append_document_begin( &update, "$set", strlen( "$set" ), &fields ) == false )
        {
            return HB_FAILURE;
        }

        if( __hb_db_append_values( &fields, _handles, e_hb_db_mode_set ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        if( bson_append_document_end( &update, &fields ) == false )
        {
            return HB_FAILURE;
        }
    }

    if( __hb_db_has_values_mode( _handles, e_hb_db_mode_inc ) == HB_TRUE )
    {
        bson_t fields;
        if( bson_append_document_begin( &update, "$inc", strlen( "$inc" ), &fields ) == false )
        {
            return HB_FAILURE;
        }

        if( __hb_db_append_values( &fields, _handles, e_hb_db_mode_inc ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        if( bson_append_document_end( &update, &fields ) == false )
        {
            return HB_FAILURE;
        }
    }

    bson_error_t error;
    if( mongoc_collection_update_one( mongo_collection, &query, &update, HB_NULLPTR, HB_NULLPTR, &error ) == false )
    {
        HB_LOG_MESSAGE_ERROR( "db", "update values: %s"
            , error.message
        );

        return HB_FAILURE;
    }

    bson_destroy( &query );
    bson_destroy( &update );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_update_values_by_name( const hb_db_client_handle_t * _client, uint32_t _puid, const char * _name, hb_uid_t _uid, const hb_db_values_handle_t * _values )
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_project_collection( _client, _puid, _name, &db_collection ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid get collection '%s'"
            , _name
        );

        return HB_FAILURE;
    }

    hb_result_t result = hb_db_update_values( db_collection, _uid, _values );

    hb_db_destroy_collection( db_collection );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_upload_script( const hb_db_collection_handle_t * _handle, const hb_sha1_t * _sha1, const void * _code, hb_size_t _codesize, const char * _source, hb_size_t _sourcesize )
{
    mongoc_collection_t * mongo_collection = _handle->mongo_collection;

    bson_t query;
    bson_init( &query );
    BSON_APPEND_BINARY( &query, "sha1", BSON_SUBTYPE_BINARY, _sha1->value, sizeof( hb_sha1_t ) );

    mongoc_cursor_t * cursor = mongoc_collection_find( mongo_collection, MONGOC_QUERY_NONE, 0, 0, 0, &query, HB_NULLPTR, HB_NULLPTR );

    bson_destroy( &query );

    const bson_t * data;
    mongoc_cursor_next( cursor, &data );

    if( data == HB_NULLPTR )
    {
        bson_t document;
        bson_init( &document );
        bson_append_binary( &document, "sha1", strlen( "sha1" ), BSON_SUBTYPE_BINARY, _sha1->value, sizeof( hb_sha1_t ) );
        bson_append_binary( &document, "script_code", strlen( "script_code" ), BSON_SUBTYPE_BINARY, _code, _codesize );
        bson_append_utf8( &document, "script_source", strlen( "script_source" ), _source, _sourcesize );

        bson_error_t insert_error;
        if( mongoc_collection_insert_one( mongo_collection, &document, HB_NULLPTR, HB_NULLPTR, &insert_error ) == false )
        {
            mongoc_cursor_destroy( cursor );

            hb_size_t sha1hexsize;
            char sha1hex[41];
            hb_base64_encode( _sha1->value, sizeof( hb_sha1_t ), sha1hex, 41, &sha1hexsize );

            HB_LOG_MESSAGE_ERROR( "db", "failed to upload script sha1: %s\nerror message: %s\n"
                , sha1hex
                , insert_error.message
            );

            return HB_FAILURE;
        }

        bson_destroy( &document );
    }

    mongoc_cursor_destroy( cursor );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
typedef struct hb_db_script_handle_t
{
    mongoc_cursor_t * cursor;
    const hb_byte_t * buffer;
    hb_size_t size;
} hb_db_script_handle_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_load_script( const hb_db_collection_handle_t * _handle, const hb_byte_t * _sha1, hb_db_script_handle_t ** _script )
{
    mongoc_collection_t * mongo_collection = _handle->mongo_collection;

    bson_t query;
    bson_init( &query );
    BSON_APPEND_BINARY( &query, "sha1", BSON_SUBTYPE_BINARY, _sha1, 20 );

    mongoc_cursor_t * cursor = mongoc_collection_find( mongo_collection, MONGOC_QUERY_NONE, 0, 0, 0, &query, HB_NULLPTR, HB_NULLPTR );

    bson_destroy( &query );

    const bson_t * data;
    mongoc_cursor_next( cursor, &data );

    bson_iter_t iter;
    if( bson_iter_init( &iter, data ) == false )
    {
        mongoc_cursor_destroy( cursor );

        return HB_FAILURE;
    }

    if( __hb_db_find_iter( data, &iter, "script_code" ) == HB_FAILURE )
    {
        mongoc_cursor_destroy( cursor );

        return HB_FAILURE;
    }

    bson_subtype_t subtype;
    uint32_t script_code_length;
    const hb_byte_t * script_code_buffer;
    bson_iter_binary( &iter, &subtype, &script_code_length, &script_code_buffer );

    if( subtype != BSON_SUBTYPE_BINARY )
    {
        mongoc_cursor_destroy( cursor );

        return HB_FAILURE;
    }

    hb_db_script_handle_t * script = HB_NEW( hb_db_script_handle_t );

    script->cursor = cursor;
    script->size = script_code_length;
    script->buffer = script_code_buffer;

    *_script = script;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
const hb_byte_t * hb_db_get_script_data( const hb_db_script_handle_t * _script, hb_size_t * _size )
{
    *_size = _script->size;

    const hb_byte_t * buffer = _script->buffer;

    return buffer;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_close_script( hb_db_script_handle_t * _script )
{
    mongoc_cursor_t * cursor = _script->cursor;

    mongoc_cursor_destroy( cursor );

    HB_DELETE( _script );
}
//////////////////////////////////////////////////////////////////////////