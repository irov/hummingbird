#ifndef HB_JSON_H_
#define HB_JSON_H_

#include "hb_config/hb_config.h"

typedef struct hb_json_handle_t hb_json_handle_t;

hb_result_t hb_json_create( const void * _data, hb_size_t _size, void * _pool, hb_size_t _capacity, hb_json_handle_t ** _handle );
hb_result_t hb_json_mapping( void * _data, hb_size_t _size, void * _pool, hb_size_t _capacity, hb_json_handle_t ** _handle );
hb_result_t hb_json_load( const char * _file, void * _pool, hb_size_t _capacity, hb_json_handle_t ** _handle );

typedef enum hb_json_type_t
{
    e_hb_json_object,
    e_hb_json_array,
    e_hb_json_string,
    e_hb_json_integer,
    e_hb_json_real,
    e_hb_json_true,
    e_hb_json_false,
    e_hb_json_null,
    e_hb_json_invalid,
} hb_json_type_t;

hb_json_type_t hb_json_get_type( const hb_json_handle_t * _handle );

hb_bool_t hb_json_is_num( const hb_json_handle_t * _handle );
hb_bool_t hb_json_is_object_empty( const hb_json_handle_t * _handle );
hb_bool_t hb_json_is_array_empty( const hb_json_handle_t * _handle );
hb_bool_t hb_json_is_array( const hb_json_handle_t * _handle );

uint32_t hb_json_array_count( const hb_json_handle_t * _handle );
hb_result_t hb_json_array_get( const hb_json_handle_t * _handle, uint32_t _index, hb_json_handle_t ** _out );

hb_result_t hb_json_get_field( const hb_json_handle_t * _handle, const char * _key, hb_json_handle_t ** _out );
uint32_t hb_json_get_fields_count( const hb_json_handle_t * _handle );

hb_result_t hb_json_to_string( const hb_json_handle_t * _handle, const char ** _value, hb_size_t * _size );
hb_result_t hb_json_to_boolean( const hb_json_handle_t * _handle, hb_bool_t * const _value );
hb_result_t hb_json_to_int16( const hb_json_handle_t * _handle, int16_t * const _value );
hb_result_t hb_json_to_int32( const hb_json_handle_t * _handle, int32_t * const _value );
hb_result_t hb_json_to_int64( const hb_json_handle_t * _handle, int64_t * const _value );
hb_result_t hb_json_to_uint16( const hb_json_handle_t * _handle, uint16_t * const _value );
hb_result_t hb_json_to_uint32( const hb_json_handle_t * _handle, uint32_t * const _value );
hb_result_t hb_json_to_uint64( const hb_json_handle_t * _handle, uint64_t * const _value );
hb_result_t hb_json_to_float( const hb_json_handle_t * _handle, float * const _value );
hb_result_t hb_json_to_double( const hb_json_handle_t * _handle, double * const _value );

hb_result_t hb_json_get_field_string( hb_json_handle_t * _handle, const char * _key, const char ** _value, hb_size_t * const _size );
hb_result_t hb_json_get_field_string_default( hb_json_handle_t * _handle, const char * _key, const char ** _value, hb_size_t * _size, const char * _default );
hb_result_t hb_json_copy_field_string( hb_json_handle_t * _handle, const char * _key, char * _value, hb_size_t _capacity );
hb_result_t hb_json_copy_field_string_default( hb_json_handle_t * _handle, const char * _key, char * _value, hb_size_t _capacity, const char * _default );
hb_result_t hb_json_get_field_int16( hb_json_handle_t * _handle, const char * _key, int16_t * _value );
hb_result_t hb_json_get_field_int16_default( hb_json_handle_t * _handle, const char * _key, int16_t * _value, int16_t _default );
hb_result_t hb_json_get_field_int32( hb_json_handle_t * _handle, const char * _key, int32_t * _value );
hb_result_t hb_json_get_field_int32_default( hb_json_handle_t * _handle, const char * _key, int32_t * _value, int32_t _default );
hb_result_t hb_json_get_field_int64( hb_json_handle_t * _handle, const char * _key, int64_t * _value );
hb_result_t hb_json_get_field_int64_default( hb_json_handle_t * _handle, const char * _key, int64_t * _value, int64_t _default );
hb_result_t hb_json_get_field_uint16( hb_json_handle_t * _handle, const char * _key, uint16_t * _value );
hb_result_t hb_json_get_field_uint16_default( hb_json_handle_t * _handle, const char * _key, uint16_t * _value, uint16_t _default );
hb_result_t hb_json_get_field_uint32( hb_json_handle_t * _handle, const char * _key, uint32_t * _value );
hb_result_t hb_json_get_field_uint32_default( hb_json_handle_t * _handle, const char * _key, uint32_t * _value, uint32_t _default );
hb_result_t hb_json_get_field_uint64( hb_json_handle_t * _handle, const char * _key, uint64_t * _value );
hb_result_t hb_json_get_field_uint64_default( hb_json_handle_t * _handle, const char * _key, uint64_t * _value, uint64_t _default );

hb_result_t hb_json_update( hb_json_handle_t * _base, hb_json_handle_t * _update, void * _pool, hb_size_t _capacity, hb_json_handle_t ** _result );
hb_result_t hb_json_dumps( hb_json_handle_t * _handle, char * _buffer, hb_size_t _capacity, hb_size_t * _size );

typedef hb_result_t(*hb_json_object_visitor_t)(hb_size_t _index, const hb_json_handle_t * _key, const hb_json_handle_t * _value, void * _ud);
hb_result_t hb_json_object_foreach( const hb_json_handle_t * _handle, hb_json_object_visitor_t _visitor, void * _ud );

typedef hb_result_t( *hb_json_array_visitor_t )(hb_size_t _index, const hb_json_handle_t * _value, void * _ud);
hb_result_t hb_json_array_foreach( const hb_json_handle_t * _handle, hb_json_array_visitor_t _visitor, void * _ud );

#endif
