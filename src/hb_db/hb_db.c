#include "hb_db.h"

#include "hb_config/hb_config.h"
#include "hb_log/hb_log.h"
#include "hb_utils/hb_sha1.h"
#include "hb_utils/hb_base64.h"

#include "mongoc/mongoc.h"

//////////////////////////////////////////////////////////////////////////
mongoc_client_t * g_mongo_client = HB_NULLPTR;
//////////////////////////////////////////////////////////////////////////
int hb_db_initialze( const char * _name, const char * _uri )
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

        return 0;
    }

    mongoc_client_t * client = mongoc_client_new_from_uri( uri );

    if( client == HB_NULLPTR )
    {
        mongoc_uri_destroy( uri );

        return 0;
    }

    mongoc_uri_destroy( uri );

    mongoc_client_set_appname( client, _name );

    g_mongo_client = client;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_finalize()
{
    mongoc_client_destroy( g_mongo_client );
    g_mongo_client = HB_NULLPTR;

    mongoc_cleanup();
}
//////////////////////////////////////////////////////////////////////////
int hb_db_get_collection( const char * _db, const char * _name, hb_db_collection_handle_t * _collection )
{
    mongoc_collection_t * collection = mongoc_client_get_collection( g_mongo_client, _db, _name );

    _collection->handle = collection;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_collection_destroy( hb_db_collection_handle_t * _collection )
{
    mongoc_collection_t * mongo_collection = (mongoc_collection_t *)_collection->handle;

    mongoc_collection_destroy( mongo_collection );
}
//////////////////////////////////////////////////////////////////////////
int hb_db_new_document( hb_db_collection_handle_t * _collection, uint8_t _newoid[12] )
{
    mongoc_collection_t * mongo_collection = (mongoc_collection_t *)_collection->handle;

    bson_oid_t oid;
    bson_oid_init( &oid, HB_NULLPTR );

    bson_t query;
    bson_init( &query );

    BSON_APPEND_OID( &query, "_id", &oid );

    bson_error_t error;
    if( mongoc_collection_insert_one( mongo_collection, &query, HB_NULLPTR, HB_NULLPTR, &error ) == false )
    {
        return 0;
    }

    bson_destroy( &query );

    memcpy( _newoid, oid.bytes, 12 );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_make_string_value( const char * _field, size_t _fieldlength, const char * _value, size_t _valuelength, hb_db_value_handle_t * _handle )
{
    _handle->handle = HB_NULLPTR;
    _handle->type = e_hb_db_string;
    _handle->field = _field;
    _handle->length_field = _fieldlength == ~0U ? strlen( _field ) : _fieldlength;
    _handle->value_string = _value;
    _handle->length_string = _valuelength == ~0U ? strlen( _value ) : _valuelength;
}
//////////////////////////////////////////////////////////////////////////
void hb_make_buffer_value( const char * _field, size_t _fieldlength, const void * _value, size_t _valuelength, hb_db_value_handle_t * _handle )
{
    _handle->handle = HB_NULLPTR;
    _handle->type = e_hb_db_binary;
    _handle->field = _field;
    _handle->length_field = _fieldlength == ~0U ? strlen( _field ) : _fieldlength;
    _handle->value_binary = _value;
    _handle->length_binary = _valuelength;
}
//////////////////////////////////////////////////////////////////////////
void hb_make_int64_value( const char * _field, size_t _fieldlength, int64_t _value, hb_db_value_handle_t * _handle )
{
    _handle->handle = HB_NULLPTR;
    _handle->type = e_hb_db_int64;
    _handle->field = _field;
    _handle->length_field = _fieldlength == ~0U ? strlen( _field ) : _fieldlength;
    _handle->value_int64 = _value;
}
//////////////////////////////////////////////////////////////////////////
int hb_db_get_value( hb_db_collection_handle_t * _collection, const uint8_t _oid[12], const char * _field, hb_db_value_type_e _type, hb_db_value_handle_t * _handle )
{
    mongoc_collection_t * mongo_collection = (mongoc_collection_t *)_collection->handle;

    bson_oid_t oid;
    bson_oid_init_from_data( &oid, _oid );

    bson_t query;
    bson_init( &query );
    BSON_APPEND_OID( &query, "_id", &oid );

    bson_t fields;
    bson_init( &fields );

    BSON_APPEND_INT32( &fields, _field, 1 );

    mongoc_cursor_t * cursor = mongoc_collection_find( mongo_collection, MONGOC_QUERY_NONE, 0, 0, 0, &query, &fields, HB_NULLPTR );

    bson_destroy( &query );
    bson_destroy( &fields );

    const bson_t * data;
    if( mongoc_cursor_next( cursor, &data ) == false )
    {
        mongoc_cursor_destroy( cursor );

        return 0;
    }

    bson_iter_t iter;
    if( bson_iter_init( &iter, data ) == false )
    {
        mongoc_cursor_destroy( cursor );

        return 0;
    }

    _handle->handle = cursor;
    _handle->type = _type;
    _handle->field = _field;
    _handle->length_field = strlen( _field );

    if( bson_iter_find( &iter, _field ) == false )
    {
        mongoc_cursor_destroy( cursor );

        return 0;
    }

    switch( _type )
    {
    case e_hb_db_int64:
        {
            _handle->value_int64 = bson_iter_int64( &iter );
        }break;
    case e_hb_db_string:
        {
            uint32_t utf8_length;
            const char * utf8_value = bson_iter_utf8( &iter, &utf8_length );

            _handle->length_string = utf8_length;
            _handle->value_string = utf8_value;
        }break;
    case e_hb_db_binary:
        {
            bson_subtype_t binary_subtype;
            uint32_t binary_length;
            const uint8_t * binary_value;
            bson_iter_binary( &iter, &binary_subtype, &binary_length, &binary_value );

            _handle->length_binary = binary_length;
            _handle->value_binary = binary_value;
        }break;
    default:
        {
            return 0;
        }break;
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_db_new_value( hb_db_collection_handle_t * _collection, const uint8_t _oid[12], const hb_db_value_handle_t * _handle )
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

    switch( _handle->type )
    {
    case e_hb_db_int64:
        {
            bson_append_int64( &fields, _handle->field, _handle->length_field, _handle->value_int64 );
        }break;
    case e_hb_db_string:
        {
            bson_append_symbol( &fields, _handle->field, _handle->length_field, _handle->value_string, _handle->length_string );
        }break;
    case e_hb_db_binary:
        {
            bson_append_binary( &fields, _handle->field, _handle->length_field, BSON_SUBTYPE_BINARY, _handle->value_binary, _handle->length_binary );
        }break;
    default:
        {
            return 0;
        }break;
    }

    bson_append_document_end( &update, &fields );

    bson_error_t error;
    if( mongoc_collection_update_one( mongo_collection, &query, &update, HB_NULLPTR, HB_NULLPTR, &error ) == false )
    {
        return 0;
    }

    bson_destroy( &query );
    bson_destroy( &update );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_db_new_values( hb_db_collection_handle_t * _collection, const uint8_t _oid[12], const hb_db_value_handle_t * _handles, size_t _count )
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

    for( uint32_t index = 0; index != _count; ++index )
    {
        const hb_db_value_handle_t * handle = _handles + index;

        switch( handle->type )
        {
        case e_hb_db_int64:
            {
                bson_append_int64( &fields, handle->field, handle->length_field, handle->value_int64 );
            }break;
        case e_hb_db_string:
            {
                bson_append_symbol( &fields, handle->field, handle->length_field, handle->value_string, handle->length_string );
            }break;
        case e_hb_db_binary:
            {
                bson_append_binary( &fields, handle->field, handle->length_field, BSON_SUBTYPE_BINARY, handle->value_binary, handle->length_binary );
            }break;
        default:
            {
                return 0;
            }break;
        }
    }

    bson_append_document_end( &update, &fields );

    bson_error_t error;
    if( mongoc_collection_update_one( mongo_collection, &query, &update, HB_NULLPTR, HB_NULLPTR, &error ) == false )
    {
        return 0;
    }

    bson_destroy( &query );
    bson_destroy( &update );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_db_value_destroy( hb_db_value_handle_t * _value )
{
    mongoc_cursor_t * cursor = (mongoc_cursor_t *)_value->handle;

    mongoc_cursor_destroy( cursor );
}
//////////////////////////////////////////////////////////////////////////
int hb_db_upload_file( hb_db_collection_handle_t * _collection, const uint8_t * _sha1, const void * _buffer, size_t _size )
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

            char sha1hex[41];
            hb_base64_decode( _sha1, 20, sha1hex, 41, HB_NULLPTR );

            hb_log_message( "db", HB_LOG_ERROR,
                "failed to insert: %s\n"
                "error message: %s\n",
                sha1hex,
                insert_error.message );

            return 0;
        }

        bson_destroy( &document );
    }

    mongoc_cursor_destroy( cursor );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_db_load_file( hb_db_collection_handle_t * _collection, const uint8_t * _sha1, hb_db_file_handle_t * _handle )
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

        return 0;
    }

    if( bson_iter_find( &iter, "data" ) == false )
    {
        mongoc_cursor_destroy( cursor );

        return 0;
    }

    bson_subtype_t subtype;
    uint32_t length;
    const uint8_t * buffer;
    bson_iter_binary( &iter, &subtype, &length, &buffer );

    if( subtype != BSON_SUBTYPE_BINARY )
    {
        mongoc_cursor_destroy( cursor );

        return 0;
    }

    _handle->handle = cursor;
    _handle->length = length;
    _handle->buffer = buffer;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_db_close_file( hb_db_file_handle_t * _handle )
{
    mongoc_cursor_t * cursor = (mongoc_cursor_t *)_handle->handle;

    mongoc_cursor_destroy( cursor );

    return 1;
}