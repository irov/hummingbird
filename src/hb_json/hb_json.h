#ifndef HB_JSON_H_
#define HB_JSON_H_

#include "hb_config/hb_config.h"

typedef struct hb_json_handle_t hb_json_handle_t;

hb_result_t hb_json_create( const void * _data, size_t _size, hb_json_handle_t ** _handle );
void hb_json_destroy( hb_json_handle_t * _handle );

hb_result_t hb_json_load( const char * _file, hb_json_handle_t ** _handle );

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
} hb_json_type_t;

hb_result_t hb_json_get_field( hb_json_handle_t * _handle, const char * _key, hb_json_handle_t ** _out );
uint32_t hb_json_get_fields_count( hb_json_handle_t * _handle );
hb_json_type_t hb_json_get_type( hb_json_handle_t * _handle );
hb_result_t hb_json_to_oid16( hb_json_handle_t * _handle, hb_oid16_t * _oid );
hb_result_t hb_json_to_string( hb_json_handle_t * _handle, const char ** _value, size_t * _size );
hb_result_t hb_json_to_integer( hb_json_handle_t * _handle, int64_t * _value );
hb_result_t hb_json_to_real( hb_json_handle_t * _handle, double * _value );

hb_result_t hb_json_get_field_oid16( hb_json_handle_t * _handle, const char * _key, hb_oid16_t * _oid );
hb_result_t hb_json_get_field_string( hb_json_handle_t * _handle, const char * _key, const char ** _value, size_t * _size, const char * _default );
hb_result_t hb_json_copy_field_string( hb_json_handle_t * _handle, const char * _key, char * _value, size_t _capacity );
hb_result_t hb_json_get_field_integer( hb_json_handle_t * _handle, const char * _key, int64_t * _value, int64_t _default );

hb_result_t hb_json_update( hb_json_handle_t * _base, hb_json_handle_t * _update );
hb_result_t hb_json_dumps( hb_json_handle_t * _handle, char * _buffer, size_t _capacity, size_t * _size );

typedef void(*hb_json_visitor_t)(const char * _key, hb_json_handle_t * _value, void * _ud);
void hb_json_foreach( hb_json_handle_t * _handle, hb_json_visitor_t _visitor, void * _ud );

#endif
