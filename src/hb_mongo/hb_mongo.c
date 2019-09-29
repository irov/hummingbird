#include "hb_mongo.h"

#include "hb_config/hb_config.h"
#include "hb_log/hb_log.h"

#include "mongoc/mongoc.h"

mongoc_client_t * g_mongo_client = HB_NULLPTR;

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

void hb_mongo_finalize()
{
    mongoc_client_destroy( g_mongo_client );
    g_mongo_client = HB_NULLPTR;

    mongoc_cleanup();
}

int hb_mongo_get_collection( const char * _db, const char * _name, hb_mongo_collection_handler_t * _collection )
{
    mongoc_collection_t * collection = mongoc_client_get_collection( g_mongo_client, _db, _name );

    _collection->handler = collection;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_mongo_get_value( hb_mongo_collection_handler_t * _collection, const char * _id, hb_mongo_value_handler_t * _value )
{
    mongoc_collection_t * mongo_collection = (mongoc_collection_t *)_collection->handler;

    bson_oid_t oid;
    bson_oid_init_from_string( &oid, _id );

    bson_t * query = BCON_NEW( "$query",
        "{",
        "_id",
        BCON_OID( &oid ),
        "}" );

    mongoc_cursor_t * cursor = mongoc_collection_find( mongo_collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL );

    bson_destroy( query );

    const bson_t * value;
    mongoc_cursor_next( cursor, &value );

    _value->handler1 = cursor;
    _value->handler2 = value;
    _value->value = HB_NULLPTR;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_mongo_value_destroy( hb_mongo_value_handler_t * _value )
{
    mongoc_cursor_t * cursor = (mongoc_cursor_t *)_value->handler1;

    mongoc_cursor_destroy( cursor );
}
//////////////////////////////////////////////////////////////////////////
const char * hb_mongo_value_lock( hb_mongo_value_handler_t * _value )
{
    const bson_t * mongo_value = (const bson_t *)_value->handler2;

    const char * str = bson_as_canonical_extended_json( mongo_value, NULL );

    _value->value = str;

    return str;
}
//////////////////////////////////////////////////////////////////////////
void hb_mongo_value_unlock( hb_mongo_value_handler_t * _value )
{
    bson_free( (char *)_value->value );
}
//////////////////////////////////////////////////////////////////////////