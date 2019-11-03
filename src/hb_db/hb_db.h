#ifndef HB_DB_H_
#define HB_DB_H_

#include "hb_config/hb_config.h"

#include "hb_utils/hb_time.h"

hb_result_t hb_db_initialze( const char * _name, const char * _uri, uint16_t _port );
void hb_db_finalize();

typedef struct hb_db_collection_handle_t
{
    void * handle;
} hb_db_collection_handle_t;

hb_result_t hb_db_get_collection( const char * _db, const char * _name, hb_db_collection_handle_t * _collection );
void hb_db_destroy_collection( const hb_db_collection_handle_t * _collection );

hb_result_t hb_db_set_collection_expire( const hb_db_collection_handle_t * _collection, const char * _field, uint32_t _expire );

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

typedef struct hb_db_value_handle_t
{
    const void * handle;
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

void hb_db_make_int32_value( const char * _field, size_t _fieldlength, int32_t _value, hb_db_value_handle_t * _handle );
void hb_db_make_int64_value( const char * _field, size_t _fieldlength, int64_t _value, hb_db_value_handle_t * _handle );
void hb_db_make_symbol_value( const char * _field, size_t _fieldlength, const char * _buffer, size_t _bufferlength, hb_db_value_handle_t * _handle );
void hb_db_make_binary_value( const char * _field, size_t _fieldlength, const void * _buffer, size_t _bufferlength, hb_db_value_handle_t * _handle );
void hb_db_make_time_value( const char * _field, size_t _fieldlength, hb_time_t _time, hb_db_value_handle_t * _handle );
void hb_db_make_oid_value( const char * _field, size_t _fieldlength, const uint8_t * _oid, hb_db_value_handle_t * _handle );

hb_result_t hb_db_new_document( const hb_db_collection_handle_t * _collection, const hb_db_value_handle_t * _handles, uint32_t _count, hb_oid_t _newoid );

hb_result_t hb_db_find_oid( const hb_db_collection_handle_t * _collection, const hb_db_value_handle_t * _handles, uint32_t _count, hb_oid_t _oid, hb_bool_t * _exist );
hb_result_t hb_db_count_values( const hb_db_collection_handle_t * _collection, const hb_db_value_handle_t * _handles, uint32_t _count, uint32_t * _founds );
hb_result_t hb_db_get_value( const hb_db_collection_handle_t * _collection, const hb_oid_t _oid, const char * _field, hb_db_value_handle_t * _handles );
hb_result_t hb_db_get_values( const hb_db_collection_handle_t * _collection, const hb_oid_t _oid, const char ** _fields, uint32_t _count, hb_db_value_handle_t * _handles );
hb_result_t hb_db_update_values( const hb_db_collection_handle_t * _collection, const hb_oid_t _oid, const hb_db_value_handle_t * _handles, uint32_t _count );
void hb_db_destroy_values( const hb_db_value_handle_t * _values, uint32_t _count );

hb_result_t hb_db_upload_script( const hb_db_collection_handle_t * _collection, const uint8_t * _sha1, const void * _code, size_t _codesize, const char * _source, size_t _sourcesize );

typedef struct hb_db_script_handle_t
{
    const void * handle;
    size_t script_code_length;
    const uint8_t * script_code_buffer;
} hb_db_script_handle_t;

hb_result_t hb_db_load_script( const hb_db_collection_handle_t * _collection, const uint8_t * _sha1, hb_db_script_handle_t * _handle );
void hb_db_close_script( hb_db_script_handle_t * _handle );

#endif
