#ifndef HB_DB_H_
#define HB_DB_H_

#include "hb_config/hb_config.h"

int hb_db_initialze( const char * _name, const char * _uri );
void hb_db_finalize();

typedef struct hb_db_collection_handle_t
{
    void * handle;
} hb_db_collection_handle_t;

int hb_db_get_collection( const char * _db, const char * _name, hb_db_collection_handle_t * _collection );
void hb_db_collection_destroy( hb_db_collection_handle_t * _collection );

int hb_db_new_document( hb_db_collection_handle_t * _collection, uint8_t _newoid[12] );

typedef enum hb_db_value_type_e
{    
    e_hb_db_int64,
    e_hb_db_string,
    e_hb_db_binary,
} hb_db_value_type_e;

typedef struct hb_db_value_handle_t
{
    const void * handle;
    hb_db_value_type_e type;

    const char * field;
    size_t length_field;
    
    const char * value_string;
    size_t length_string;

    const void * value_binary;
    size_t length_binary;

    int64_t value_int64;
} hb_db_value_handle_t;

void hb_make_int64_value( const char * _field, size_t _fieldlength, int64_t _value, hb_db_value_handle_t * _handle );
void hb_make_string_value( const char * _field, size_t _fieldlength, const char * _value, size_t _valuelength, hb_db_value_handle_t * _handle );
void hb_make_buffer_value( const char * _field, size_t _fieldlength, const void * _value, size_t _valuelength, hb_db_value_handle_t * _handle );


int hb_db_get_value( hb_db_collection_handle_t * _collection, const uint8_t _oid[12], const char * _field, hb_db_value_type_e _type, hb_db_value_handle_t * _handle );
int hb_db_new_value( hb_db_collection_handle_t * _collection, const uint8_t _oid[12], const hb_db_value_handle_t * _handle );
int hb_db_new_values( hb_db_collection_handle_t * _collection, const uint8_t _oid[12], const hb_db_value_handle_t * _handles, size_t _count );
void hb_db_value_destroy( hb_db_value_handle_t * _value );

int hb_db_upload_file( hb_db_collection_handle_t * _collection, const uint8_t * _sha1, const void * _buffer, size_t _size );

typedef struct hb_db_file_handle_t
{
    const void * handle;
    size_t length;
    const uint8_t * buffer;
} hb_db_file_handle_t;

int hb_db_load_file( hb_db_collection_handle_t * _collection, const uint8_t * _sha1, hb_db_file_handle_t * _handle );
int hb_db_close_file( hb_db_file_handle_t * _handle );

#endif
