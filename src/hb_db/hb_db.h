#ifndef HB_DB_H_
#define HB_DB_H_

#include "hb_config/hb_config.h"

#include "hb_utils/hb_time.h"

typedef struct hb_db_handle_t hb_db_handle_t;

hb_result_t hb_db_initialze( const char * _uri, uint16_t _port, hb_db_handle_t ** _handle );
void hb_db_finalize( hb_db_handle_t * _handle );

typedef struct hb_db_client_handle_t hb_db_client_handle_t;

hb_result_t hb_db_create_client( hb_db_handle_t * _handle, hb_db_client_handle_t ** _client );
void hb_db_destroy_client( hb_db_handle_t * _handle, hb_db_client_handle_t * _client );

typedef struct hb_db_collection_handle_t hb_db_collection_handle_t;

hb_result_t hb_db_get_collection( const hb_db_client_handle_t * _client, const char * _db, const char * _name, hb_db_collection_handle_t ** _collection );
void hb_db_destroy_collection( hb_db_collection_handle_t * _collection );

hb_result_t hb_db_set_collection_expire( const hb_db_collection_handle_t * _collection, const char * _field, uint32_t _expire );

typedef struct hb_db_values_handle_t hb_db_values_handle_t;

hb_result_t hb_db_create_values( hb_db_values_handle_t ** _values );
void hb_db_destroy_values( hb_db_values_handle_t * _values );

void hb_db_copy_values( hb_db_values_handle_t * _values, const hb_db_values_handle_t * _source );

void hb_db_make_uid_value( hb_db_values_handle_t * _values, const char * _field, size_t _fieldlength, hb_uid_t _value );
void hb_db_make_int32_value( hb_db_values_handle_t * _values, const char * _field, size_t _fieldlength, int32_t _value );
void hb_db_make_int64_value( hb_db_values_handle_t * _values, const char * _field, size_t _fieldlength, int64_t _value );
void hb_db_make_string_value( hb_db_values_handle_t * _values, const char * _field, size_t _fieldlength, const char * _buffer, size_t _bufferlength );
void hb_db_make_binary_value( hb_db_values_handle_t * _values, const char * _field, size_t _fieldlength, const void * _buffer, size_t _bufferlength );
void hb_db_make_time_value( hb_db_values_handle_t * _values, const char * _field, size_t _fieldlength, hb_time_t _time );
void hb_db_make_oid_value( hb_db_values_handle_t * _values, const char * _field, size_t _fieldlength, const hb_oid_t * _oid );
void hb_db_make_sha1_value( hb_db_values_handle_t * _values, const char * _field, size_t _fieldlength, const hb_sha1_t * _sha1 );

hb_result_t hb_db_get_uid_value( const hb_db_values_handle_t * _values, uint32_t _index, hb_uid_t * _value );
hb_result_t hb_db_get_int32_value( const hb_db_values_handle_t * _values, uint32_t _index, int32_t * _value );
hb_result_t hb_db_get_uint32_value( const hb_db_values_handle_t * _values, uint32_t _index, uint32_t * _value );
hb_result_t hb_db_get_int64_value( const hb_db_values_handle_t * _values, uint32_t _index, int64_t * _value );
hb_result_t hb_db_get_string_value( const hb_db_values_handle_t * _values, uint32_t _index, const char ** _value, size_t * _length );
hb_result_t hb_db_copy_string_value( const hb_db_values_handle_t * _values, uint32_t _index, char * _value, size_t _capacity );
hb_result_t hb_db_get_binary_value( const hb_db_values_handle_t * _values, uint32_t _index, const void ** _buffer, size_t * _length );
hb_result_t hb_db_copy_binary_value( const hb_db_values_handle_t * _values, uint32_t _index, void * _buffer, size_t _capacity );
hb_result_t hb_db_get_time_value( const hb_db_values_handle_t * _values, uint32_t _index, hb_time_t * _value );

hb_result_t hb_db_new_document( const hb_db_collection_handle_t * _collection, const hb_db_values_handle_t * _values, hb_oid_t * _newoid );
hb_result_t hb_db_new_document_by_name( const hb_db_client_handle_t * _client, const char * _name, const hb_db_values_handle_t * _values, hb_oid_t * _newoid );

hb_result_t hb_db_find_oid( const hb_db_collection_handle_t * _collection, const hb_db_values_handle_t * _query, hb_oid_t * _oid, hb_bool_t * _exist );
hb_result_t hb_db_find_oid_by_name( const hb_db_client_handle_t * _client, const char * _name, const hb_db_values_handle_t * _query, hb_oid_t * _oid, hb_bool_t * _exist );
hb_result_t hb_db_find_oid_with_values( const hb_db_collection_handle_t * _collection, const hb_db_values_handle_t * _query, hb_oid_t * _oid, const char ** _fields, uint32_t _fieldcount, hb_db_values_handle_t ** _values, hb_bool_t * _exist );
hb_result_t hb_db_find_oid_with_values_by_name( const hb_db_client_handle_t * _client, const char * _name, const hb_db_values_handle_t * _query, hb_oid_t * _oid, const char ** _fields, uint32_t _fieldcount, hb_db_values_handle_t ** _values, hb_bool_t * _exist );
hb_result_t hb_db_select_values( const hb_db_collection_handle_t * _collection, const hb_db_values_handle_t * _query, const char ** _fields, uint32_t _fieldcount, hb_db_values_handle_t ** _values, uint32_t _limit, uint32_t * _exists );
hb_result_t hb_db_count_values( const hb_db_collection_handle_t * _collection, const hb_db_values_handle_t * _query, uint32_t * _founds );
hb_result_t hb_db_gets_values( const hb_db_collection_handle_t * _collection, const hb_oid_t * _oids, uint32_t _oidcount, const char ** _fields, uint32_t _fieldscount, hb_db_values_handle_t ** _values );
hb_result_t hb_db_gets_values_by_name( const hb_db_client_handle_t * _client, const char * _name, const hb_oid_t * _oids, uint32_t _oidcount, const char ** _fields, uint32_t _fieldscount, hb_db_values_handle_t ** _values );
hb_result_t hb_db_get_values( const hb_db_collection_handle_t * _collection, const hb_oid_t * _oid, const char ** _fields, uint32_t _count, hb_db_values_handle_t ** _values );
hb_result_t hb_db_get_values_by_name( const hb_db_client_handle_t * _client, const char * _name, const hb_oid_t * _oid, const char ** _fields, uint32_t _count, hb_db_values_handle_t ** _values );
hb_result_t hb_db_update_values( const hb_db_collection_handle_t * _collection, const hb_oid_t * _oid, const hb_db_values_handle_t * _values );
hb_result_t hb_db_update_values_by_name( const hb_db_client_handle_t * _client, const char * _name, const hb_oid_t * _oid, const hb_db_values_handle_t * _values );


hb_result_t hb_db_upload_script( const hb_db_collection_handle_t * _collection, const hb_sha1_t * _sha1, const void * _code, size_t _codesize, const char * _source, size_t _sourcesize );

hb_result_t hb_db_make_uid( const hb_db_collection_handle_t * _collection, const hb_oid_t * _oid, const hb_db_values_handle_t * _values, hb_uid_t * _uid );
hb_result_t hb_db_make_uid_by_name( const hb_db_client_handle_t * _client, const char * _name, const hb_oid_t * _oid, const hb_db_values_handle_t * _values, hb_uid_t * _uid );

typedef struct hb_db_script_handle_t hb_db_script_handle_t;

hb_result_t hb_db_load_script( const hb_db_collection_handle_t * _collection, const hb_byte_t * _sha1, hb_db_script_handle_t ** _script );
const hb_byte_t * hb_db_get_script_data( const hb_db_script_handle_t * _script, size_t * _size );

void hb_db_close_script( hb_db_script_handle_t * _data );

#endif
