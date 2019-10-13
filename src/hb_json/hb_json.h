#ifndef HB_JSON_H_
#define HB_JSON_H_

#include "hb_config/hb_config.h"

typedef struct hb_json_handle_t
{
    void * handle;
}hb_json_handle_t;

int hb_json_create( const void * _data, size_t _size, hb_json_handle_t * _handle );
void hb_json_destroy( hb_json_handle_t * _handle );

int hb_json_get_string( hb_json_handle_t * _handle, const char * _key, const char ** _value, size_t * _size );
int hb_json_dumpb_value( hb_json_handle_t * _handle, const char * _key, char * _buffer, size_t _capacity, size_t * _size );

int hb_json_dumpb( hb_json_handle_t * _handle, char * _buffer, size_t _capacity, size_t * _size );

#endif
