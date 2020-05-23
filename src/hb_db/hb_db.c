#include "hb_db.h"

#include "hb_memory/hb_memory.h"
#include "hb_log/hb_log.h"
#include "hb_utils/hb_sha1.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_rand.h"

#include "mongoc/mongoc.h"

//////////////////////////////////////////////////////////////////////////
#ifndef HB_DB_VALUE_HANDLE_MAX_VALUES
#define HB_DB_VALUE_HANDLE_MAX_VALUES 16
#endif
//////////////////////////////////////////////////////////////////////////
typedef enum hb_db_value_type_e
{
    e_hb_db_int32,
    e_hb_db_int64,
    e_hb_db_symbol,
    e_hb_db_utf8,
    e_hb_db_binary,
    e_hb_db_time,
    e_hb_db_oid,
} hb_db_value_type_e;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_db_value_handle_t
{    
    hb_db_value_type_e type;

    const char * field;
    size_t field_length;

    union
    {
        struct
        {
            const char * buffer;
            size_t length;
        } symbol;

        struct
        {
            const char * buffer;
            size_t length;
        } utf8;

        struct
        {
            const void * buffer;
            size_t length;
        } binary;

        int32_t i32;
        int64_t i64;
        hb_time_t time;

        const uint8_t * oid;
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
mongoc_client_pool_t * g_mongo_pool = HB_NULLPTR;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_initialze( const char * _uri, uint16_t _port )
{
    mongoc_init();

    mongoc_uri_t * mongoc_uri = mongoc_uri_new_for_host_port( _uri, _port );

    if( mongoc_uri == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_ERROR( "db", "failed to use URI: %s:%u"
            , _uri
            , _port
        );

        return HB_FAILURE;
    }

    mongoc_client_pool_t * mongo_pool = mongoc_client_pool_new( mongoc_uri );

    mongoc_uri_destroy( mongoc_uri );

    if( mongoc_client_pool_set_error_api( mongo_pool, MONGOC_ERROR_API_VERSION_2 ) == false )
    {
        HB_LOG_MESSAGE_ERROR( "db", "failed to set error api: %s:%u"
            , _uri
            , _port
        );

        return HB_FAILURE;
    }    

    bson_t ping;
    bson_init( &ping );

    bson_append_int32( &ping, "ping", sizeof( "ping" ) - 1, 1 );

    bson_error_t error;

    mongoc_client_t * mongo_client = mongoc_client_pool_pop( mongo_pool );

    bool mongoc_ping = mongoc_client_command_simple( mongo_client, "admin", &ping, NULL, NULL, &error );

    mongoc_client_pool_push( mongo_pool, mongo_client );

    bson_destroy( &ping );

    if( mongoc_ping == false )
    {
        mongoc_client_destroy( mongo_client );

        return HB_FAILURE;
    }

    g_mongo_pool = mongo_pool;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_finalize()
{
    mongoc_client_pool_destroy( g_mongo_pool );
    g_mongo_pool = HB_NULLPTR;
    
    mongoc_cleanup();
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_create_client( hb_db_client_handle_t ** _handle )
{
    hb_db_client_handle_t * handle = HB_NEW( hb_db_client_handle_t );

    mongoc_client_t * mongo_client = mongoc_client_pool_pop( g_mongo_pool );
    handle->mongo_client = mongo_client;

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_destroy_client( hb_db_client_handle_t * _handle )
{
    mongoc_client_t * mongo_client = _handle->mongo_client;
    mongoc_client_pool_push( g_mongo_pool, mongo_client );

    HB_DELETE( _handle );
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
        HB_LOG_MESSAGE_ERROR( "db"
            , "failed to set collection expire: %s\nerror message: %s\n"
            , mongoc_collection_get_name( mongo_collection )
            , error.message
        );

        return HB_FAILURE;
    }

    bson_destroy( &keys );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_db_append_values( bson_t * _bson, const hb_db_values_handle_t * _handles )
{
    uint32_t value_count = _handles->value_count;

    for( uint32_t index = 0; index != value_count; ++index )
    {
        const hb_db_value_handle_t * handle = _handles->values + index;

        switch( handle->type )
        {
        case e_hb_db_int32:
            {
                bson_append_int32( _bson, handle->field, handle->field_length, handle->u.i32 );
            }break;
        case e_hb_db_int64:
            {
                bson_append_int64( _bson, handle->field, handle->field_length, handle->u.i64 );
            }break;
        case e_hb_db_symbol:
            {
                bson_append_symbol( _bson, handle->field, handle->field_length, handle->u.symbol.buffer, handle->u.symbol.length );
            }break;
        case e_hb_db_binary:
            {
                bson_append_binary( _bson, handle->field, handle->field_length, BSON_SUBTYPE_BINARY, handle->u.binary.buffer, handle->u.binary.length );
            }break;
        case e_hb_db_time:
            {
                bson_append_time_t( _bson, handle->field, handle->field_length, (time_t)handle->u.time );
            }break;
        case e_hb_db_oid:
            {
                bson_oid_t oid;
                bson_oid_init_from_data( &oid, handle->u.oid );

                bson_append_oid( _bson, handle->field, handle->field_length, &oid );
            }break;
        default:
            {
                return HB_FAILURE;
            }break;
        }
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_new_document( const hb_db_collection_handle_t * _collection, const hb_db_values_handle_t * _values, hb_oid_t * _newoid )
{
    mongoc_collection_t * mongo_collection = _collection->mongo_collection;

    bson_oid_t oid;
    bson_oid_init( &oid, HB_NULLPTR );

    bson_t query;
    bson_init( &query );

    BSON_APPEND_OID( &query, "_id", &oid );

    if( __hb_db_append_values( &query, _values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    bson_error_t error;
    if( mongoc_collection_insert_one( mongo_collection, &query, HB_NULLPTR, HB_NULLPTR, &error ) == false )
    {
        return HB_FAILURE;
    }

    bson_destroy( &query );

    memcpy( _newoid->value, oid.bytes, sizeof( hb_oid_t ) );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_new_document_by_name( const hb_db_client_handle_t * _client, const char * _name, const hb_db_values_handle_t * _values, hb_oid_t * _newoid )
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_collection( _client, "hb", _name, &db_collection ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid get collection '%s'"
            , _name
        );

        return HB_FAILURE;
    }

    hb_result_t result = hb_db_new_document( db_collection, _values, _newoid );

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

    for( size_t index = 0; index != _source->value_count; ++index )
    {
        _values->values[index] = _source->values[index];
    }

    _values->value_count += _source->value_count;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_uid_value( hb_db_values_handle_t * _values, const char * _field, size_t _fieldlength, hb_uid_t _value )
{
    hb_db_value_handle_t * value = _values->values + _values->value_count;
    ++_values->value_count;

    value->type = e_hb_db_int32;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.i32 = (int32_t)_value;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_int32_value( hb_db_values_handle_t * _values, const char * _field, size_t _fieldlength, int32_t _value )
{
    hb_db_value_handle_t * value = _values->values + _values->value_count;
    ++_values->value_count;

    value->type = e_hb_db_int32;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.i32 = _value;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_int64_value( hb_db_values_handle_t * _values, const char * _field, size_t _fieldlength, int64_t _value )
{
    hb_db_value_handle_t * value = _values->values + _values->value_count;
    ++_values->value_count;

    value->type = e_hb_db_int64;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.i64 = _value;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_symbol_value( hb_db_values_handle_t * _values, const char * _field, size_t _fieldlength, const char * _buffer, size_t _bufferlength )
{
    hb_db_value_handle_t * value = _values->values + _values->value_count;
    ++_values->value_count;

    value->type = e_hb_db_symbol;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.symbol.buffer = _buffer;
    value->u.symbol.length = _bufferlength == HB_UNKNOWN_STRING_SIZE ? strlen( _buffer ) : _bufferlength;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_binary_value( hb_db_values_handle_t * _handles, const char * _field, size_t _fieldlength, const void * _buffer, size_t _bufferlength )
{
    hb_db_value_handle_t * value = _handles->values + _handles->value_count;
    ++_handles->value_count;

    value->type = e_hb_db_binary;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.binary.buffer = _buffer;
    value->u.binary.length = _bufferlength;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_time_value( hb_db_values_handle_t * _values, const char * _field, size_t _fieldlength, hb_time_t _time )
{
    hb_db_value_handle_t * value = _values->values + _values->value_count;
    ++_values->value_count;

    value->type = e_hb_db_time;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.time = _time;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_oid_value( hb_db_values_handle_t * _values, const char * _field, size_t _fieldlength, const hb_oid_t * _oid )
{
    hb_db_value_handle_t * value = _values->values + _values->value_count;
    ++_values->value_count;

    value->type = e_hb_db_oid;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.oid = _oid->value;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_sha1_value( hb_db_values_handle_t * _values, const char * _field, size_t _fieldlength, const hb_sha1_t * _sha1 )
{
    hb_db_value_handle_t * value = _values->values + _values->value_count;
    ++_values->value_count;

    value->type = e_hb_db_binary;
    value->field = _field;
    value->field_length = _fieldlength == HB_UNKNOWN_STRING_SIZE ? strlen( _field ) : _fieldlength;
    value->u.binary.buffer = _sha1->value;
    value->u.binary.length = sizeof( hb_sha1_t );
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
hb_result_t hb_db_get_symbol_value( const hb_db_values_handle_t * _values, uint32_t _index, const char ** _value, size_t * _length )
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
hb_result_t hb_db_get_binary_value( const hb_db_values_handle_t * _values, uint32_t _index, const void ** _buffer, size_t * _length )
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
hb_result_t hb_db_copy_binary_value( const hb_db_values_handle_t * _values, uint32_t _index, void * _buffer, size_t _size )
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

    if( value->u.binary.length != _size )
    {
        return HB_FAILURE;
    }

    memcpy( _buffer, value->u.binary.buffer, _size );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t __hb_db_find_iter( const bson_t * _data, bson_iter_t * _iter, const char * _key )
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
hb_result_t hb_db_find_oid( const hb_db_collection_handle_t * _handle, const hb_db_values_handle_t * _query, hb_oid_t * _oid, hb_bool_t * _exist )
{
    mongoc_collection_t * mongo_collection = _handle->mongo_collection;

    bson_t query;
    bson_init( &query );

    if( __hb_db_append_values( &query, _query ) == HB_FAILURE )
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

    const bson_oid_t * oid = bson_iter_oid( &iter );

    if( _oid != HB_NULLPTR )
    {
        memcpy( _oid->value, oid->bytes, sizeof( hb_oid_t ) );
    }

    mongoc_cursor_destroy( cursor );

    *_exist = HB_TRUE;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_find_oid_by_name( const hb_db_client_handle_t * _client, const char * _name, const hb_db_values_handle_t * _query, hb_oid_t * _oid, hb_bool_t * _exist )
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_collection( _client, "hb", _name, &db_collection ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid get collection '%s'"
            , _name
        );

        return HB_FAILURE;
    }

    hb_result_t result = hb_db_find_oid( db_collection, _query, _oid, _exist );

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
        return HB_FAILURE;
    }

    bson_type_t type = bson_iter_type( _iter );

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
    case BSON_TYPE_OID:
        {
            _value->type = e_hb_db_oid;

            const bson_oid_t * value_oid = bson_iter_oid( _iter );

            _value->u.oid = value_oid->bytes;
        }break;
    default:
        {
            return HB_FAILURE;
        }break;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_find_oid_with_values( const hb_db_collection_handle_t * _handle, const hb_db_values_handle_t * _query, hb_oid_t * _oid, const char ** _fields, uint32_t _fieldcount, hb_db_values_handle_t ** _values, hb_bool_t * _exist )
{
    mongoc_collection_t * mongo_collection = _handle->mongo_collection;

    bson_t query;
    bson_init( &query );

    if( __hb_db_append_values( &query, _query ) == HB_FAILURE )
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

    if( _oid != HB_NULLPTR )
    {
        if( __hb_db_find_iter( data, &iter, "_id" ) == HB_FAILURE )
        {
            mongoc_cursor_destroy( cursor );

            return HB_FAILURE;
        }

        const bson_oid_t * oid = bson_iter_oid( &iter );

        memcpy( _oid->value, oid->bytes, sizeof( hb_oid_t ) );
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
    
    bson_error_t error;
    if( mongoc_cursor_error( cursor, &error ) )
    {
        HB_LOG_MESSAGE_ERROR( "db", "find with values values error occurred: %s"
            , error.message 
        );

        mongoc_cursor_destroy( cursor );

        return HB_FAILURE;
    }

    *_values = values;
    *_exist = HB_TRUE;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_find_oid_with_values_by_name( const hb_db_client_handle_t * _client, const char * _name, const hb_db_values_handle_t * _query, hb_oid_t * _oid, const char ** _fields, uint32_t _fieldcount, hb_db_values_handle_t ** _values, hb_bool_t * _exist )
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_collection( _client, "hb", _name, &db_collection ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid get collection '%s'"
            , _name
        );

        return HB_FAILURE;
    }

    hb_result_t result = hb_db_find_oid_with_values( db_collection, _query, _oid, _fields, _fieldcount, _values, _exist );

    hb_db_destroy_collection( db_collection );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_select_values( const hb_db_collection_handle_t * _handle, const hb_db_values_handle_t * _query, const char ** _fields, uint32_t _fieldcount, hb_db_values_handle_t ** _values, uint32_t _limit, uint32_t * _exists )
{
    mongoc_collection_t * mongo_collection = _handle->mongo_collection;

    bson_t query;
    bson_init( &query );

    if( __hb_db_append_values( &query, _query ) == HB_FAILURE )
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
        HB_LOG_MESSAGE_ERROR( "db", "select values error occurred: %s"
            , error.message 
        );

        mongoc_cursor_destroy( cursor );

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

    if( __hb_db_append_values( &filter, _query ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    bson_error_t error;
    int64_t count = mongoc_collection_count_documents( mongo_collection, &filter, HB_NULLPTR, HB_NULLPTR, HB_NULLPTR, &error );

    bson_destroy( &filter );

    if( count < 0 )
    {
        return HB_FAILURE;
    }

    *_founds = (uint32_t)count;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_gets_values( const hb_db_collection_handle_t * _collection, const hb_oid_t * _oids, uint32_t _oidcount, const char ** _fields, uint32_t _fieldscount, hb_db_values_handle_t ** _values )
{
    if( _oidcount == 0 )
    {
        return HB_SUCCESSFUL;
    }

    mongoc_collection_t * mongo_collection = _collection->mongo_collection;

    bson_t query;
    bson_init( &query );

    if( _oidcount == 1 )
    {
        const hb_oid_t * oid = _oids + 0;

        bson_oid_t boid;
        bson_oid_init_from_data( &boid, oid->value );

        BSON_APPEND_OID( &query, "_id", &boid );
    }
    else
    {
        bson_t query_id;
        bson_append_document_begin( &query, "_id", -1, &query_id );

        bson_t query_in;
        bson_append_array_begin( &query_id, "$in", -1, &query_in );

        for( uint32_t index = 0; index != _oidcount; ++index )
        {
            const hb_oid_t * oid = _oids + index;

            bson_oid_t boid;
            bson_oid_init_from_data( &boid, oid->value );

            BSON_APPEND_OID( &query_in, "$oid", &boid );
        }

        bson_append_array_end( &query_id, &query_in );
        bson_append_document_end( &query, &query_id );
    }

    bson_t fields;
    bson_init( &fields );

    if( _oidcount != 0 )
    {
        BSON_APPEND_INT32( &fields, "_id", 1 );
    }

    for( uint32_t index = 0; index != _fieldscount; ++index )
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

    for( uint32_t index_oid = 0; index_oid != _oidcount; ++index_oid )
    {
        const bson_t * data;
        if( mongoc_cursor_next( cursor, &data ) == false )
        {
            mongoc_cursor_destroy( cursor );

            return HB_FAILURE;
        }

        bson_iter_t iter;
        if( bson_iter_init( &iter, data ) == false )
        {
            mongoc_cursor_destroy( cursor );

            return HB_FAILURE;
        }

        uint32_t correct_index_oid = ~0U;

        if( _oidcount != 0 )
        {
            if( __hb_db_find_iter( data, &iter, "_id" ) == HB_FAILURE )
            {
                mongoc_cursor_destroy( cursor );

                return HB_FAILURE;
            }

            const bson_oid_t * oid = bson_iter_oid( &iter );

            for( uint32_t index_correct = 0; index_correct != _oidcount; ++index_correct )
            {
                const hb_oid_t * correct_oid = _oids + index_correct;

                if( memcmp( correct_oid->value, oid->bytes, 12 ) == 0 )
                {
                    correct_index_oid = index_correct;
                }
            }

            if( correct_index_oid == ~0U )
            {
                mongoc_cursor_destroy( cursor );

                return HB_FAILURE;
            }
        }
        else
        {
            correct_index_oid = 0;
        }

        for( uint32_t index_field = 0; index_field != _fieldscount; ++index_field )
        {
            hb_db_value_handle_t * value = values->values + correct_index_oid * _fieldscount + index_field;
            ++values->value_count;

            const char * field = _fields[index_field];

            if( __hb_db_get_bson_value( value, data, &iter, field ) == HB_FAILURE )
            {
                mongoc_cursor_destroy( cursor );

                return HB_FAILURE;
            }
        }
    }

    bson_error_t error;
    if( mongoc_cursor_error( cursor, &error ) )
    {
        HB_LOG_MESSAGE_ERROR( "db", "find with values values error occurred: %s"
            , error.message
        );

        mongoc_cursor_destroy( cursor );

        return HB_FAILURE;
    }

    *_values = values;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_gets_values_by_name( const hb_db_client_handle_t * _client, const char * _name, const hb_oid_t * _oids, uint32_t _oidcount, const char ** _fields, uint32_t _fieldscount, hb_db_values_handle_t ** _values )
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_collection( _client, "hb", _name, &db_collection ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid get collection '%s'"
            , _name
        );

        return HB_FAILURE;
    }

    hb_result_t result = hb_db_gets_values( db_collection, _oids, _oidcount, _fields, _fieldscount, _values );

    hb_db_destroy_collection( db_collection );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_values( const hb_db_collection_handle_t * _collection, const hb_oid_t * _oid, const char ** _fields, uint32_t _count, hb_db_values_handle_t ** _values )
{
    hb_result_t result = hb_db_gets_values( _collection, _oid, 1, _fields, _count, _values );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_values_by_name( const hb_db_client_handle_t * _client, const char * _name, const hb_oid_t * _oid, const char ** _fields, uint32_t _count, hb_db_values_handle_t ** _values )
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_collection( _client, "hb", _name, &db_collection ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid get collection '%s'"
            , _name
        );

        return HB_FAILURE;
    }

    hb_result_t result = hb_db_get_values( db_collection, _oid, _fields, _count, _values );

    hb_db_destroy_collection( db_collection );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_update_values( const hb_db_collection_handle_t * _collection, const hb_oid_t * _oid, const hb_db_values_handle_t * _handles )
{
    mongoc_collection_t * mongo_collection = _collection->mongo_collection;

    bson_oid_t oid;
    bson_oid_init_from_data( &oid, _oid->value );

    bson_t query;
    bson_init( &query );
    BSON_APPEND_OID( &query, "_id", &oid );

    bson_t update;
    bson_init( &update );

    bson_t fields;
    bson_append_document_begin( &update, "$set", strlen( "$set" ), &fields );

    if( __hb_db_append_values( &fields, _handles ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    bson_append_document_end( &update, &fields );

    bson_error_t error;
    if( mongoc_collection_update_one( mongo_collection, &query, &update, HB_NULLPTR, HB_NULLPTR, &error ) == false )
    {
        return HB_FAILURE;
    }

    bson_destroy( &query );
    bson_destroy( &update );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_update_values_by_name( const hb_db_client_handle_t * _client, const char * _name, const hb_oid_t * _oid, const hb_db_values_handle_t * _values )
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_collection( _client, "hb", _name, &db_collection ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid get collection '%s'"
            , _name
        );

        return HB_FAILURE;
    }

    hb_result_t result = hb_db_update_values( db_collection, _oid, _values );

    hb_db_destroy_collection( db_collection );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_upload_script( const hb_db_collection_handle_t * _handle, const hb_sha1_t * _sha1, const void * _code, size_t _codesize, const char * _source, size_t _sourcesize )
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

            size_t sha1hexsize;
            char sha1hex[41];
            hb_base64_encode( _sha1->value, sizeof( hb_sha1_t ), sha1hex, 41, &sha1hexsize );

            HB_LOG_MESSAGE_ERROR( "db"
                , "failed to insert: %s\nerror message: %s\n"
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
hb_result_t hb_db_make_uid( const hb_db_collection_handle_t * _collection, const hb_oid_t * _oid, const hb_db_values_handle_t * _values, hb_uid_t * _uid )
{
    hb_uid_t uid = 0;
    uint32_t founds = 0;
    for( ; founds != 1; )
    {
        do
        {
            uid = hb_rand_time();
            uid &= 0x7fffffff;
        } while( uid == 0 );

        hb_db_values_handle_t * update_values;
        hb_db_create_values( &update_values );

        hb_db_make_uid_value( update_values, "uid", HB_UNKNOWN_STRING_SIZE, uid );

        if( hb_db_update_values( _collection, _oid, update_values ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
        
        hb_db_destroy_values( update_values );

        hb_db_values_handle_t * count_values;

        if( hb_db_create_values( &count_values ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_copy_values( count_values, _values );
        hb_db_make_uid_value( count_values, "uid", HB_UNKNOWN_STRING_SIZE, uid );

        if( hb_db_count_values( _collection, count_values, &founds ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_destroy_values( count_values );

        if( founds == 0 )
        {
            return HB_FAILURE;
        }
    }

    *_uid = uid;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_make_uid_by_name( const hb_db_client_handle_t * _client, const char * _name, const hb_oid_t * _oid, const hb_db_values_handle_t * _values, hb_uid_t * _uid )
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_collection( _client, "hb", _name, &db_collection ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "db", "invalid get collection '%s'"
            , _name
        );

        return HB_FAILURE;
    }

    hb_result_t result = hb_db_make_uid( db_collection, _oid, _values, _uid );

    hb_db_destroy_collection( db_collection );

    return result;
}
//////////////////////////////////////////////////////////////////////////
typedef struct hb_db_script_handle_t
{
    mongoc_cursor_t * cursor;
    const hb_byte_t * buffer;
    size_t size;
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
const hb_byte_t * hb_db_get_script_data( const hb_db_script_handle_t * _script, size_t * _size )
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