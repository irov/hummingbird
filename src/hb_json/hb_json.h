#ifndef HB_JSON_H_
#define HB_JSON_H_

#include "hb_config/hb_config.h"

typedef struct hb_json_handle_t
{
    void * handle;
}hb_json_handle_t;

hb_result_t hb_json_create( const void * _data, size_t _size, hb_json_handle_t * _handle );
void hb_json_destroy( hb_json_handle_t * _handle );

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

hb_result_t hb_json_get_field( hb_json_handle_t * _handle, const char * _key, hb_json_handle_t * _out );
hb_json_type_t hb_json_get_type( hb_json_handle_t * _handle );
hb_result_t hb_json_to_string( hb_json_handle_t * _handle, const char ** _value, size_t * _size );

hb_result_t hb_json_get_field_string( hb_json_handle_t * _handle, const char * _key, const char ** _value, size_t * _size );


hb_result_t hb_json_dumpb_value( hb_json_handle_t * _handle, char * _buffer, size_t _capacity, size_t * _size );
hb_result_t hb_json_dumpb( hb_json_handle_t * _handle, char * _buffer, size_t _capacity, size_t * _size );

#endif
