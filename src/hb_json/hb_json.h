#ifndef HB_JSON_H_
#define HB_JSON_H_

#include "hb_config/hb_config.h"

typedef struct hb_json_handle_t hb_json_handle_t;

hb_result_t hb_json_create( const void * _data, hb_size_t _size, void * _pool, hb_size_t _capacity, hb_json_handle_t ** _handle );
hb_result_t hb_json_mapping( const void * _data, hb_size_t _size, void * _pool, hb_size_t _capacity, hb_json_handle_t ** _handle );
hb_result_t hb_json_load( const char * _file, void * _pool, hb_size_t _capacity, hb_json_handle_t ** _handle );
void hb_json_free( hb_json_handle_t * _handle );

typedef enum hb_json_type_e
{
    e_hb_json_null,
    e_hb_json_false,
    e_hb_json_true,
    e_hb_json_integer,
    e_hb_json_real,
    e_hb_json_string,
    e_hb_json_array,
    e_hb_json_object,
} hb_json_type_e;

typedef struct hb_json_string_t
{
    const char * value;
    hb_size_t size;
} hb_json_string_t;

hb_json_type_e hb_json_get_type( const hb_json_handle_t * _handle );

hb_bool_t hb_json_is_array( const hb_json_handle_t * _handle );
hb_bool_t hb_json_is_array_empty( const hb_json_handle_t * _handle );
hb_size_t hb_json_get_array_size( const hb_json_handle_t * _handle );
hb_result_t hb_json_array_get_element( const hb_json_handle_t * _handle, uint32_t _index, const hb_json_handle_t ** _out );

hb_bool_t hb_json_is_object( const hb_json_handle_t * _handle );
hb_bool_t hb_json_is_object_empty( const hb_json_handle_t * _handle );
hb_size_t hb_json_get_object_size( const hb_json_handle_t * _handle );
hb_result_t hb_json_get_field( const hb_json_handle_t * _handle, const char * _key, const hb_json_handle_t ** _out );

hb_result_t hb_json_to_boolean( const hb_json_handle_t * _handle, hb_bool_t * const _value );
hb_result_t hb_json_to_int16( const hb_json_handle_t * _handle, int16_t * const _value );
hb_result_t hb_json_to_int32( const hb_json_handle_t * _handle, int32_t * const _value );
hb_result_t hb_json_to_int64( const hb_json_handle_t * _handle, int64_t * const _value );
hb_result_t hb_json_to_uint16( const hb_json_handle_t * _handle, uint16_t * const _value );
hb_result_t hb_json_to_uint32( const hb_json_handle_t * _handle, uint32_t * const _value );
hb_result_t hb_json_to_uint64( const hb_json_handle_t * _handle, uint64_t * const _value );
hb_result_t hb_json_to_size_t( const hb_json_handle_t * _handle, hb_size_t * const _value );
hb_result_t hb_json_to_float( const hb_json_handle_t * _handle, float * const _value );
hb_result_t hb_json_to_double( const hb_json_handle_t * _handle, double * const _value );
hb_result_t hb_json_to_string( const hb_json_handle_t * _handle, hb_json_string_t * const _value );

hb_bool_t hb_json_is_string( const hb_json_handle_t * _handle );
hb_result_t hb_json_copy_string( const hb_json_handle_t * _handle, char * _value, hb_size_t _capacity, hb_size_t * const _size );

hb_result_t hb_json_get_field_boolean( const hb_json_handle_t * _handle, const char * _key, hb_bool_t * const _value );
void hb_json_get_field_boolean_default( const hb_json_handle_t * _handle, const char * _key, hb_bool_t * const _value, hb_bool_t _default );
hb_result_t hb_json_get_field_int16( const hb_json_handle_t * _handle, const char * _key, int16_t * const _value );
void hb_json_get_field_int16_default( const hb_json_handle_t * _handle, const char * _key, int16_t * const _value, int16_t _default );
hb_result_t hb_json_get_field_int32( const hb_json_handle_t * _handle, const char * _key, int32_t * const _value );
void hb_json_get_field_int32_default( const hb_json_handle_t * _handle, const char * _key, int32_t * const _value, int32_t _default );
hb_result_t hb_json_get_field_int64( const hb_json_handle_t * _handle, const char * _key, int64_t * const _value );
void hb_json_get_field_int64_default( const hb_json_handle_t * _handle, const char * _key, int64_t * const _value, int64_t _default );
hb_result_t hb_json_get_field_uint16( const hb_json_handle_t * _handle, const char * _key, uint16_t * const _value );
void hb_json_get_field_uint16_default( const hb_json_handle_t * _handle, const char * _key, uint16_t * const _value, uint16_t _default );
hb_result_t hb_json_get_field_uint32( const hb_json_handle_t * _handle, const char * _key, uint32_t * const _value );
void hb_json_get_field_uint32_default( const hb_json_handle_t * _handle, const char * _key, uint32_t * const _value, uint32_t _default );
hb_result_t hb_json_get_field_uint64( const hb_json_handle_t * _handle, const char * _key, uint64_t * const _value );
void hb_json_get_field_uint64_default( const hb_json_handle_t * _handle, const char * _key, uint64_t * const _value, uint64_t _default );
hb_result_t hb_json_get_field_size_t( const hb_json_handle_t * _handle, const char * _key, hb_size_t * const _value );
void hb_json_get_field_size_t_default( const hb_json_handle_t * _handle, const char * _key, hb_size_t * const _value, hb_size_t _default );
hb_result_t hb_json_get_field_string( const hb_json_handle_t * _handle, const char * _key, hb_json_string_t * const _value );
void hb_json_get_field_string_default( const hb_json_handle_t * _handle, const char * _key, hb_json_string_t * const _value, const char * _default );

hb_result_t hb_json_copy_field_string( const hb_json_handle_t * _handle, const char * _key, char * _value, hb_size_t _capacity );
hb_result_t hb_json_copy_field_string_default( const hb_json_handle_t * _handle, const char * _key, char * _value, hb_size_t _capacity, const char * _default );

hb_result_t hb_json_update( hb_json_handle_t * _base, hb_json_handle_t * _update, void * _pool, hb_size_t _capacity, hb_json_handle_t ** _result );
hb_result_t hb_json_dump( const hb_json_handle_t * _handle, char * _buffer, hb_size_t _capacity, hb_size_t * const _size );
hb_result_t hb_json_dump_string( const char * _value, char * _buffer, hb_size_t _capacity, hb_size_t * const _size );

typedef hb_result_t( *hb_json_object_visit_t )(hb_size_t _index, const hb_json_handle_t * _key, const hb_json_handle_t * _value, void * _ud);
hb_result_t hb_json_visit_object( const hb_json_handle_t * _handle, hb_json_object_visit_t _visitor, void * _ud );

typedef hb_result_t( *hb_json_array_visit_t )(hb_size_t _index, const hb_json_handle_t * _value, void * _ud);
hb_result_t hb_json_visit_array( const hb_json_handle_t * _handle, hb_json_array_visit_t _visitor, void * _ud );

#endif
