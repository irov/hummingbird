#ifndef HB_DB_H_
#define HB_DB_H_

#include "hb_config/hb_config.h"

#include "hb_json/hb_json.h"

#include "hb_utils/hb_time.h"

typedef struct hb_db_handle_t hb_db_handle_t;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_initialze( const char * _uri, const char * _host, uint16_t _port, hb_db_handle_t ** _handle );
void hb_db_finalize( hb_db_handle_t * _handle );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
typedef struct hb_db_client_handle_t hb_db_client_handle_t;

hb_result_t hb_db_create_client( hb_db_handle_t * _handle, hb_db_client_handle_t ** _client );
void hb_db_destroy_client( hb_db_handle_t * _handle, hb_db_client_handle_t * _client );

typedef struct hb_db_collection_handle_t hb_db_collection_handle_t;

hb_result_t hb_db_get_collection( const hb_db_client_handle_t * _client, const char * _db, const char * _name, hb_db_collection_handle_t ** _collection );
hb_result_t hb_db_get_project_collection( const hb_db_client_handle_t * _client, hb_uid_t _uid, const char * _name, hb_db_collection_handle_t ** _collection );
void hb_db_destroy_collection( hb_db_collection_handle_t * _collection );

hb_result_t hb_db_set_collection_expire( const hb_db_collection_handle_t * _collection, const char * _field, uint32_t _expire );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
typedef struct hb_db_values_handle_t hb_db_values_handle_t;

hb_result_t hb_db_create_values( hb_db_values_handle_t ** _values );
void hb_db_destroy_values( hb_db_values_handle_t * _values );

void hb_db_copy_values( hb_db_values_handle_t * _values, const hb_db_values_handle_t * _source );

void hb_db_make_uid_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, hb_uid_t _value );
void hb_db_make_int32_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, int32_t _value );
void hb_db_make_int64_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, int64_t _value );
void hb_db_make_string_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, const char * _string, hb_size_t _stringlength );
void hb_db_make_binary_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, const void * _buffer, hb_size_t _bufferlength );
void hb_db_make_time_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, hb_time_t _time );
void hb_db_make_sha1_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, const hb_sha1_t * _sha1 );

void hb_db_inc_int32_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, int32_t _value );

hb_result_t hb_db_make_dictionary_value( hb_db_values_handle_t * _values, const char * _field, hb_size_t _fieldlength, hb_db_values_handle_t ** _dictionary );

hb_result_t hb_db_get_uid_value( const hb_db_values_handle_t * _values, uint32_t _index, hb_uid_t * _value );
hb_result_t hb_db_get_int32_value( const hb_db_values_handle_t * _values, uint32_t _index, int32_t * _value );
hb_result_t hb_db_get_uint32_value( const hb_db_values_handle_t * _values, uint32_t _index, uint32_t * _value );
hb_result_t hb_db_get_int64_value( const hb_db_values_handle_t * _values, uint32_t _index, int64_t * _value );
hb_result_t hb_db_get_string_value( const hb_db_values_handle_t * _values, uint32_t _index, const char ** _value, hb_size_t * _length );
hb_result_t hb_db_copy_string_value( const hb_db_values_handle_t * _values, uint32_t _index, char * _value, hb_size_t _capacity );
hb_result_t hb_db_get_binary_value( const hb_db_values_handle_t * _values, uint32_t _index, const void ** _buffer, hb_size_t * _length );
hb_result_t hb_db_copy_binary_value( const hb_db_values_handle_t * _values, uint32_t _index, void * _buffer, hb_size_t _capacity );
hb_result_t hb_db_get_time_value( const hb_db_values_handle_t * _values, uint32_t _index, hb_time_t * _value );
hb_result_t hb_db_get_json_value( const hb_db_values_handle_t * _values, uint32_t _index, void * _pool, hb_size_t _capacity, const hb_json_handle_t ** _value );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_new_document( const hb_db_collection_handle_t * _collection, const hb_db_values_handle_t * _values, hb_uid_t * _newoid );
hb_result_t hb_db_new_document_by_name( const hb_db_client_handle_t * _client, uint32_t _puid, const char * _name, const hb_db_values_handle_t * _values, hb_uid_t * _newuid );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_exist_uid( const hb_db_collection_handle_t * _collection, hb_uid_t _uid, hb_bool_t * _exist );
hb_result_t hb_db_exist_project_uid( const hb_db_client_handle_t * _client, hb_uid_t _uid, hb_bool_t * _exist );
hb_result_t hb_db_exist_uid_by_name( const hb_db_client_handle_t * _client, uint32_t _puid, const char * _name, hb_uid_t _uid, hb_bool_t * _exist );
hb_result_t hb_db_find_uid( const hb_db_collection_handle_t * _collection, const hb_db_values_handle_t * _query, hb_uid_t * _uid, hb_bool_t * _exist );
hb_result_t hb_db_find_uid_by_name( const hb_db_client_handle_t * _client, uint32_t _puid, const char * _name, const hb_db_values_handle_t * _query, hb_uid_t * _uid, hb_bool_t * _exist );
hb_result_t hb_db_find_uid_with_values( const hb_db_collection_handle_t * _collection, const hb_db_values_handle_t * _query, hb_uid_t * _uid, const char ** _fields, uint32_t _fieldcount, hb_db_values_handle_t ** _values, hb_bool_t * _exist );
hb_result_t hb_db_find_uid_with_values_by_name( const hb_db_client_handle_t * _client, uint32_t _puid, const char * _name, const hb_db_values_handle_t * _query, hb_uid_t * _uid, const char ** _fields, uint32_t _fieldcount, hb_db_values_handle_t ** _values, hb_bool_t * _exist );
hb_result_t hb_db_select_values( const hb_db_collection_handle_t * _collection, const hb_db_values_handle_t * _query, const char ** _fields, uint32_t _fieldcount, hb_db_values_handle_t ** _values, uint32_t _limit, uint32_t * _exists );
hb_result_t hb_db_count_values( const hb_db_collection_handle_t * _collection, const hb_db_values_handle_t * _query, uint32_t * _founds );
hb_result_t hb_db_gets_values( const hb_db_collection_handle_t * _collection, const hb_uid_t * _uids, uint32_t _uidcount, const char ** _fields, uint32_t _fieldscount, hb_db_values_handle_t ** _values, hb_bool_t * _exist );
hb_result_t hb_db_gets_values_by_name( const hb_db_client_handle_t * _client, uint32_t _puid, const char * _name, const hb_uid_t * _uids, uint32_t _uidcount, const char ** _fields, uint32_t _fieldscount, hb_db_values_handle_t ** _values, hb_bool_t * _exist );
hb_result_t hb_db_get_values( const hb_db_collection_handle_t * _collection, hb_uid_t _uid, const char ** _fields, uint32_t _count, hb_db_values_handle_t ** _values, hb_bool_t * _exist );
hb_result_t hb_db_get_values_by_name( const hb_db_client_handle_t * _client, uint32_t _puid, const char * _name, hb_uid_t _uid, const char ** _fields, uint32_t _count, hb_db_values_handle_t ** _values, hb_bool_t * _exist );
hb_result_t hb_db_update_values( const hb_db_collection_handle_t * _collection, hb_uid_t _uid, const hb_db_values_handle_t * _values );
hb_result_t hb_db_update_values_by_name( const hb_db_client_handle_t * _client, uint32_t _puid, const char * _name, hb_uid_t _uid, const hb_db_values_handle_t * _values );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_db_upload_script( const hb_db_collection_handle_t * _collection, const hb_sha1_t * _sha1, const void * _code, hb_size_t _codesize, const char * _source, hb_size_t _sourcesize );

typedef struct hb_db_script_handle_t hb_db_script_handle_t;

hb_result_t hb_db_load_script( const hb_db_collection_handle_t * _collection, const hb_byte_t * _sha1, hb_db_script_handle_t ** _script );
const hb_byte_t * hb_db_get_script_data( const hb_db_script_handle_t * _script, hb_size_t * _size );

void hb_db_close_script( hb_db_script_handle_t * _data );

#endif
