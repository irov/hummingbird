#ifndef HB_JSON_H_
#define HB_JSON_H_

#include "hb_config/hb_config.h"

typedef struct hb_json_handler_t
{
    void * handler;
}hb_json_handler_t;

int hb_json_create( const void * _data, size_t _size, hb_json_handler_t * _handler );
void hb_json_destroy( hb_json_handler_t * _handler );

int hb_json_get_string( hb_json_handler_t * _handler, const char * _key, const char ** _value, size_t * _size );
int hb_json_dumpb_value( hb_json_handler_t * _handler, const char * _key, char * _buffer, size_t _capacity, size_t * _size );

#endif
