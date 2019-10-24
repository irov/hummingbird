#include "hb_db.h"

#include "hb_config/hb_config.h"
#include "hb_log/hb_log.h"
#include "hb_utils/hb_sha1.h"
#include "hb_utils/hb_base64.h"

#include "mongoc/mongoc.h"

//////////////////////////////////////////////////////////////////////////
mongoc_client_t * g_mongo_client = HB_NULLPTR;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_initialze( const char * _name, const char * _uri )
{
    mongoc_init();

    bson_error_t uri_error;
    mongoc_uri_t * uri = mongoc_uri_new_with_error( _uri, &uri_error );

    if( uri == HB_NULLPTR )
    {
        hb_log_message( "db", HB_LOG_ERROR,
            "failed to parse URI: %s\n"
            "error message:       %s\n",
            _uri,
            uri_error.message );

        return HB_FAILURE;
    }

    mongoc_client_t * client = mongoc_client_new_from_uri( uri );

    if( client == HB_NULLPTR )
    {
        mongoc_uri_destroy( uri );

        return HB_FAILURE;
    }

    mongoc_uri_destroy( uri );

    mongoc_client_set_appname( client, _name );

    g_mongo_client = client;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_finalize()
{
    mongoc_client_destroy( g_mongo_client );
    g_mongo_client = HB_NULLPTR;

    mongoc_cleanup();
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_collection( const char * _db, const char * _name, hb_db_collection_handle_t * _collection )
{
    mongoc_collection_t * collection = mongoc_client_get_collection( g_mongo_client, _db, _name );

    _collection->handle = collection;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_destroy_collection( hb_db_collection_handle_t * _collection )
{
    mongoc_collection_t * mongo_collection = (mongoc_collection_t *)_collection->handle;

    mongoc_collection_destroy( mongo_collection );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_set_collection_expire( hb_db_collection_handle_t * _collection, const char * _field, uint32_t _expire )
{
    mongoc_collection_t * mongo_collection = (mongoc_collection_t *)_collection->handle;

    mongoc_index_opt_t opt;
    mongoc_index_opt_init( &opt );

    opt.expire_after_seconds = _expire;

    bson_t keys;
    bson_init( &keys );
    bson_append_int64( &keys, _field, strlen( _field ), 1 );

    bson_error_t error;
    if( mongoc_collection_create_index( mongo_collection, &keys, &opt, &error ) == false )
    {
        hb_log_message( "db", HB_LOG_ERROR,
            "failed to set collection expire: %s\n"
            "error message:       %s\n",
            mongoc_collection_get_name( mongo_collection ),
            error.message );

        return HB_FAILURE;
    }

    bson_destroy( &keys );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_db_append_values( bson_t * _bson, const hb_db_value_handle_t * _handles, uint32_t _count )
{
    for( uint32_t index = 0; index != _count; ++index )
    {
        const hb_db_value_handle_t * handle = _handles + index;

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
        case e_hb_db_utf8:
            {
                bson_append_symbol( _bson, handle->field, handle->field_length, handle->u.utf8.buffer, handle->u.utf8.length );
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
hb_result_t hb_db_new_document( hb_db_collection_handle_t * _collection, const hb_db_value_handle_t * _handle, uint32_t _count, hb_oid_t _newoid )
{
    mongoc_collection_t * mongo_collection = (mongoc_collection_t *)_collection->handle;

    bson_oid_t oid;
    bson_oid_init( &oid, HB_NULLPTR );

    bson_t query;
    bson_init( &query );

    BSON_APPEND_OID( &query, "_id", &oid );

    __hb_db_append_values( &query, _handle, _count );

    bson_error_t error;
    if( mongoc_collection_insert_one( mongo_collection, &query, HB_NULLPTR, HB_NULLPTR, &error ) == false )
    {
        return HB_FAILURE;
    }

    bson_destroy( &query );

    memcpy( _newoid, oid.bytes, 12 );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_int32_value( const char * _field, size_t _fieldlength, int32_t _value, hb_db_value_handle_t * _handle )
{
    _handle->handle = HB_NULLPTR;
    _handle->type = e_hb_db_int32;
    _handle->field = _field;
    _handle->field_length = _fieldlength == ~0U ? strlen( _field ) : _fieldlength;
    _handle->u.i32 = _value;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_int64_value( const char * _field, size_t _fieldlength, int64_t _value, hb_db_value_handle_t * _handle )
{
    _handle->handle = HB_NULLPTR;
    _handle->type = e_hb_db_int64;
    _handle->field = _field;
    _handle->field_length = _fieldlength == ~0U ? strlen( _field ) : _fieldlength;
    _handle->u.i64 = _value;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_utf8_value( const char * _field, size_t _fieldlength, const char * _buffer, size_t _bufferlength, hb_db_value_handle_t * _handle )
{
    _handle->handle = HB_NULLPTR;
    _handle->type = e_hb_db_utf8;
    _handle->field = _field;
    _handle->field_length = _fieldlength == ~0U ? strlen( _field ) : _fieldlength;
    _handle->u.utf8.buffer = _buffer;
    _handle->u.utf8.length = _bufferlength == ~0U ? strlen( _buffer ) : _bufferlength;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_binary_value( const char * _field, size_t _fieldlength, const void * _buffer, size_t _bufferlength, hb_db_value_handle_t * _handle )
{
    _handle->handle = HB_NULLPTR;
    _handle->type = e_hb_db_binary;
    _handle->field = _field;
    _handle->field_length = _fieldlength == ~0U ? strlen( _field ) : _fieldlength;
    _handle->u.binary.buffer = _buffer;
    _handle->u.binary.length = _bufferlength;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_time_value( const char * _field, size_t _fieldlength, hb_time_t _time, hb_db_value_handle_t * _handle )
{
    _handle->handle = HB_NULLPTR;
    _handle->type = e_hb_db_time;
    _handle->field = _field;
    _handle->field_length = _fieldlength == ~0U ? strlen( _field ) : _fieldlength;
    _handle->u.time = _time;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_make_oid_value( const char * _field, size_t _fieldlength, const uint8_t * _oid, hb_db_value_handle_t * _handle )
{
    _handle->handle = HB_NULLPTR;
    _handle->type = e_hb_db_oid;
    _handle->field = _field;
    _handle->field_length = _fieldlength == ~0U ? strlen( _field ) : _fieldlength;
    _handle->u.oid = _oid;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_find_oid( hb_db_collection_handle_t * _collection, const hb_db_value_handle_t * _handles, uint32_t _count, hb_oid_t _oid, hb_result_t * _exist )
{
    mongoc_collection_t * mongo_collection = (mongoc_collection_t *)_collection->handle;

    bson_t query;
    bson_init( &query );

    __hb_db_append_values( &query, _handles, _count );

    bson_t fields;
    bson_init( &fields );

    BSON_APPEND_INT32( &fields, "_id", 1 );

    mongoc_cursor_t * cursor = mongoc_collection_find( mongo_collection, MONGOC_QUERY_NONE, 0, 0, 0, &query, &fields, HB_NULLPTR );

    bson_destroy( &query );
    bson_destroy( &fields );

    const bson_t * data;
    if( mongoc_cursor_next( cursor, &data ) == false )
    {
        mongoc_cursor_destroy( cursor );

        *_exist = HB_FAILURE;

        return HB_SUCCESSFUL;
    }

    bson_iter_t iter;
    if( bson_iter_init( &iter, data ) == false )
    {
        mongoc_cursor_destroy( cursor );

        return HB_FAILURE;
    }

    if( bson_iter_find( &iter, "_id" ) == false )
    {
        mongoc_cursor_destroy( cursor );

        return HB_FAILURE;
    }

    const bson_oid_t * oid = bson_iter_oid( &iter );

    memcpy( _oid, oid->bytes, 12 );

    mongoc_cursor_destroy( cursor );

    *_exist = HB_SUCCESSFUL;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_count_values( hb_db_collection_handle_t * _collection, const hb_db_value_handle_t * _handles, uint32_t _count, uint32_t * _founds )
{
    mongoc_collection_t * mongo_collection = (mongoc_collection_t *)_collection->handle;

    bson_t filter;
    bson_init( &filter );

    __hb_db_append_values( &filter, _handles, _count );

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
hb_result_t hb_db_get_value( hb_db_collection_handle_t * _collection, const hb_oid_t _oid, const char * _field, hb_db_value_handle_t * _handles )
{
    const char ** fields = &_field;

    hb_result_t result = hb_db_get_values( _collection, _oid, fields, 1, _handles );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_get_values( hb_db_collection_handle_t * _collection, const hb_oid_t _oid, const char ** _fields, uint32_t _count, hb_db_value_handle_t * _handles )
{
    mongoc_collection_t * mongo_collection = (mongoc_collection_t *)_collection->handle;

    bson_oid_t oid;
    bson_oid_init_from_data( &oid, _oid );

    bson_t query;
    bson_init( &query );
    BSON_APPEND_OID( &query, "_id", &oid );

    bson_t fields;
    bson_init( &fields );

    for( uint32_t index = 0; index != _count; ++index )
    {
        BSON_APPEND_INT32( &fields, _fields[index], 1 );
    }

    mongoc_cursor_t * cursor = mongoc_collection_find( mongo_collection, MONGOC_QUERY_NONE, 0, 0, 0, &query, &fields, HB_NULLPTR );

    bson_destroy( &query );
    bson_destroy( &fields );

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

    for( uint32_t index = 0; index != _count; ++index )
    {
        hb_db_value_handle_t * handle = _handles + index;

        handle->handle = index == 0 ? cursor : HB_NULLPTR;

        const char * field = _fields[index];

        handle->field = field;
        handle->field_length = strlen( field );

        if( bson_iter_find( &iter, field ) == false )
        {
            mongoc_cursor_destroy( cursor );

            return HB_FAILURE;
        }

        bson_type_t type = bson_iter_type( &iter );

        switch( type )
        {
        case BSON_TYPE_INT32:
            {
                handle->type = e_hb_db_int32;

                handle->u.i32 = bson_iter_int32( &iter );
            }break;
        case BSON_TYPE_INT64:
            {
                handle->type = e_hb_db_int64;

                handle->u.i64 = bson_iter_int64( &iter );
            }break;
        case BSON_TYPE_SYMBOL:
            {
                handle->type = e_hb_db_utf8;

                uint32_t utf8_length;
                const char * utf8_value = bson_iter_utf8( &iter, &utf8_length );

                handle->u.utf8.length = utf8_length;
                handle->u.utf8.buffer = utf8_value;
            }break;
        case BSON_TYPE_BINARY:
            {
                handle->type = e_hb_db_binary;

                bson_subtype_t binary_subtype;
                uint32_t binary_length;
                const uint8_t * binary_buffer;
                bson_iter_binary( &iter, &binary_subtype, &binary_length, &binary_buffer );

                handle->u.binary.length = binary_length;
                handle->u.binary.buffer = binary_buffer;
            }break;
        case BSON_TYPE_DATE_TIME:
            {
                handle->type = e_hb_db_time;

                handle->u.time = bson_iter_time_t( &iter );
            }break;
        case BSON_TYPE_OID:
            {
                handle->type = e_hb_db_oid;

                const bson_oid_t * value_oid = bson_iter_oid( &iter );

                handle->u.oid = value_oid->bytes;
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
hb_result_t hb_db_update_values( hb_db_collection_handle_t * _collection, const hb_oid_t _oid, const hb_db_value_handle_t * _handles, uint32_t _count )
{
    mongoc_collection_t * mongo_collection = (mongoc_collection_t *)_collection->handle;

    bson_oid_t oid;
    bson_oid_init_from_data( &oid, _oid );

    bson_t query;
    bson_init( &query );
    BSON_APPEND_OID( &query, "_id", &oid );

    bson_t update;
    bson_init( &update );

    bson_t fields;
    bson_append_document_begin( &update, "$set", strlen( "$set" ), &fields );

    __hb_db_append_values( &fields, _handles, _count );

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
void hb_db_destroy_values( hb_db_value_handle_t * _values, uint32_t _count )
{
    for( uint32_t index = 0; index != _count; ++index )
    {
        hb_db_value_handle_t * value = _values + index;

        if( value->handle == HB_NULLPTR )
        {
            continue;
        }

        mongoc_cursor_t * cursor = (mongoc_cursor_t *)value->handle;

        mongoc_cursor_destroy( cursor );
    }
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_upload_file( hb_db_collection_handle_t * _collection, const uint8_t * _sha1, const void * _buffer, size_t _size )
{
    mongoc_collection_t * mongo_collection = (mongoc_collection_t *)_collection->handle;

    bson_t query;
    bson_init( &query );
    BSON_APPEND_BINARY( &query, "sha1", BSON_SUBTYPE_BINARY, _sha1, 20 );

    mongoc_cursor_t * cursor = mongoc_collection_find( mongo_collection, MONGOC_QUERY_NONE, 0, 0, 0, &query, HB_NULLPTR, HB_NULLPTR );

    bson_destroy( &query );

    const bson_t * data;
    mongoc_cursor_next( cursor, &data );

    if( data == HB_NULLPTR )
    {
        bson_t document;
        bson_init( &document );
        BSON_APPEND_BINARY( &document, "sha1", BSON_SUBTYPE_BINARY, _sha1, 20 );
        BSON_APPEND_BINARY( &document, "data", BSON_SUBTYPE_BINARY, _buffer, _size );

        bson_error_t insert_error;
        if( mongoc_collection_insert_one( mongo_collection, &document, HB_NULLPTR, HB_NULLPTR, &insert_error ) == false )
        {
            mongoc_cursor_destroy( cursor );

            size_t sha1hexsize;
            char sha1hex[41];
            hb_base64_encode( _sha1, 20, sha1hex, 41, &sha1hexsize );

            hb_log_message( "db", HB_LOG_ERROR,
                "failed to insert: %s\n"
                "error message: %s\n",
                sha1hex,
                insert_error.message );

            return HB_FAILURE;
        }

        bson_destroy( &document );
    }

    mongoc_cursor_destroy( cursor );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_load_file( hb_db_collection_handle_t * _collection, const uint8_t * _sha1, hb_db_file_handle_t * _handle )
{
    mongoc_collection_t * mongo_collection = (mongoc_collection_t *)_collection->handle;

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

    if( bson_iter_find( &iter, "data" ) == false )
    {
        mongoc_cursor_destroy( cursor );

        return HB_FAILURE;
    }

    bson_subtype_t subtype;
    uint32_t length;
    const uint8_t * buffer;
    bson_iter_binary( &iter, &subtype, &length, &buffer );

    if( subtype != BSON_SUBTYPE_BINARY )
    {
        mongoc_cursor_destroy( cursor );

        return HB_FAILURE;
    }

    _handle->handle = cursor;
    _handle->length = length;
    _handle->buffer = buffer;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_close_file( hb_db_file_handle_t * _handle )
{
    mongoc_cursor_t * cursor = (mongoc_cursor_t *)_handle->handle;

    mongoc_cursor_destroy( cursor );
}
//////////////////////////////////////////////////////////////////////////