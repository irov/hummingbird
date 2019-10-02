#ifndef HB_DB_H_
#define HB_DB_H_

#include "hb_config/hb_config.h"

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
    size_t length[16];
    const char * value[16];
} hb_db_value_handler_t;

int hb_db_get_value( hb_db_collection_handler_t * _collection, const char * _id, const char ** _fields, uint32_t _count, hb_db_value_handler_t * _handle );
void hb_db_value_destroy( hb_db_value_handler_t * _value );

int hb_db_upload_file( hb_db_collection_handler_t * _collection, const uint8_t * _sha1, const void * _buffer, size_t _size );

typedef struct hb_db_file_handler_t
{
    const void * handler;
    size_t length;
    const uint8_t * buffer;
} hb_db_file_handler_t;

int hb_db_load_file( hb_db_collection_handler_t * _collection, const uint8_t * _sha1, hb_db_file_handler_t * _handler );
int hb_db_close_file( hb_db_file_handler_t * _handler );

#endif
