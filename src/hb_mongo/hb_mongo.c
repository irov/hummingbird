#include "hb_mongo.h"

#include "hb_config/hb_config.h"
#include "hb_log/hb_log.h"

#include "mongoc/mongoc.h"

//////////////////////////////////////////////////////////////////////////
mongoc_client_t * g_mongo_client = HB_NULLPTR;
//////////////////////////////////////////////////////////////////////////
int hb_mongo_initialze( const char * _name, const char * _uri )
{
    mongoc_init();

    bson_error_t uri_error;
    mongoc_uri_t * uri = mongoc_uri_new_with_error( _uri, &uri_error );

    if( uri == HB_NULLPTR )
    {
        hb_log_message( HB_LOG_ERROR,
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
void hb_mongo_finalize()
{
    mongoc_client_destroy( g_mongo_client );
    g_mongo_client = HB_NULLPTR;

    mongoc_cleanup();
}
//////////////////////////////////////////////////////////////////////////
int hb_mongo_get_collection( const char * _db, const char * _name, hb_mongo_collection_handler_t * _collection )
{
    mongoc_collection_t * collection = mongoc_client_get_collection( g_mongo_client, _db, _name );

    _collection->handler = collection;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_mongo_collection_destroy( hb_mongo_collection_handler_t * _collection )
{
    mongoc_collection_t * mongo_collection = (mongoc_collection_t *)_collection->handler;

    mongoc_collection_destroy( mongo_collection );
}
//////////////////////////////////////////////////////////////////////////
int hb_mongo_get_value( hb_mongo_collection_handler_t * _collection, const char * _id, hb_mongo_value_handler_t * _value )
{
    mongoc_collection_t * mongo_collection = (mongoc_collection_t *)_collection->handler;

    bson_oid_t oid;
    bson_oid_init_from_string( &oid, _id );

    bson_t query;
    bson_init( &query );
    BSON_APPEND_OID( &query, "_id", &oid );

    //bson_t fields;
    //bson_init( &fields );
    //BSON_APPEND_INT32( &fields, "value", 1 );

    mongoc_cursor_t * cursor = mongoc_collection_find( mongo_collection, MONGOC_QUERY_NONE, 0, 0, 0, &query, HB_NULLPTR, HB_NULLPTR );

    bson_destroy( &query );
    //bson_destroy( &fields );

    const bson_t * data;
    mongoc_cursor_next( cursor, &data );

    //char * str = bson_as_canonical_extended_json( data, NULL );
    //bson_free( str );

    bson_iter_t iter;
    if( bson_iter_init( &iter, data ) == false )
    {
        mongoc_cursor_destroy( cursor );

        return 0;
    }

    if( bson_iter_find( &iter, "value" ) == false )
    {
        mongoc_cursor_destroy( cursor );

        return 0;
    }

    uint32_t length;
    const char * value = bson_iter_utf8( &iter, &length );

    _value->handler = cursor;
    _value->length = (size_t)length;
    _value->value = value;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_mongo_value_destroy( hb_mongo_value_handler_t * _value )
{
    mongoc_cursor_t * cursor = (mongoc_cursor_t *)_value->handler;

    mongoc_cursor_destroy( cursor );
}
//////////////////////////////////////////////////////////////////////////
const char * hb_mongo_value_as_utf8( hb_mongo_value_handler_t * _value, size_t * _length )
{
    if( _length != HB_NULLPTR )
    {
        *_length = _value->length;
    }

    return _value->value;
}
//////////////////////////////////////////////////////////////////////////