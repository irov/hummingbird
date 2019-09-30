#ifndef HB_DB_H_
#define HB_DB_H_

#include <stdint.h>

int hb_db_initialze( const char * _name, const char * _uri );
void hb_db_finalize();

typedef struct hb_db_collection_handler_t
{
    void * handler;
} hb_db_collection_handler_t;

int hb_db_get_collection( const char * _db, const char * _name, hb_db_collection_handler_t * _collection );
void hb_db_collection_destroy( hb_db_collection_handler_t * _collection );

typedef struct hb_db_value_handler_t
{
    const void * handler;
    size_t length;
    const char * value;
} hb_db_value_handler_t;

int hb_db_get_value( hb_db_collection_handler_t * _collection, const char * _id, hb_db_value_handler_t * _value );
void hb_db_value_destroy( hb_db_value_handler_t * _value );

const char * hb_db_value_as_utf8( hb_db_value_handler_t * _value, size_t * _length );

#endif
