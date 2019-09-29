#ifndef HB_MONGO_H_
#define HB_MONGO_H_

int hb_mongo_initialze( const char * _name, const char * _uri );
void hb_mongo_finalize();

typedef struct hb_mongo_collection_handler_t
{
    void * handler;
} hb_mongo_collection_handler_t;

int hb_mongo_get_collection( const char * _db, const char * _name, hb_mongo_collection_handler_t * _collection );

typedef struct hb_mongo_value_handler_t
{
    const void * handler1;
    const void * handler2;
    const char * value;
} hb_mongo_value_handler_t;

int hb_mongo_get_value( hb_mongo_collection_handler_t * _collection, const char * _id, hb_mongo_value_handler_t * _value );
void hb_mongo_value_destroy( hb_mongo_value_handler_t * _value );

const char * hb_mongo_value_lock( hb_mongo_value_handler_t * _value );
void hb_mongo_value_unlock( hb_mongo_value_handler_t * _value );

#endif
