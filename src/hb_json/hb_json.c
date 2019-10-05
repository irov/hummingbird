#include "hb_json.h"

#include "hb_log/hb_log.h"

#include "jansson.h"

#include <memory.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_json_load_data_t
{
    const uint8_t * buffer;
    size_t carriage;
    size_t capacity;
} hb_json_load_data_t;
//////////////////////////////////////////////////////////////////////////
static size_t __hb_json_load_callback( void * _buffer, size_t _buflen, void * _ud )
{
    hb_json_load_data_t * jd = (hb_json_load_data_t *)_ud;

    if( _buflen > jd->capacity - jd->carriage )
    {
        _buflen = jd->capacity - jd->carriage;
    }

    if( _buflen <= 0 )
    {
        return 0;
    }

    const uint8_t * jd_buffer = jd->buffer + jd->carriage;
    memcpy( _buffer, jd_buffer, _buflen );
    jd->carriage += _buflen;

    return _buflen;
}
//////////////////////////////////////////////////////////////////////////
int hb_json_create( const void * _data, size_t _size, hb_json_handler_t * _handler )
{
    hb_json_load_data_t jd;
    jd.buffer = (const uint8_t *)(_data);
    jd.carriage = 0;
    jd.capacity = _size;

    json_error_t er;
    json_t * jroot = json_load_callback( &__hb_json_load_callback, &jd, 0, &er );

    if( jroot == HB_NULLPTR )
    {
        hb_log_message( "json", HB_LOG_ERROR, "json '%s' error line [%d] column [%d] position [%s]: %s"
            , er.source
            , er.line
            , er.column
            , er.position
            , er.text
        );

        return 0;
    }

    _handler->handler = jroot;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_json_destroy( hb_json_handler_t * _handler )
{
    json_t * jroot = (json_t *)_handler->handler;

    json_decref( jroot );
}
//////////////////////////////////////////////////////////////////////////
int hb_json_get_string( hb_json_handler_t * _handler, const char * _key, const char ** _value, size_t * _size )
{
    json_t * jroot = (json_t *)_handler->handler;

    json_t * jvalue = json_object_get( jroot, _key );

    if( jvalue == HB_NULLPTR )
    {
        return 0;
    }

    const char * value = json_string_value( jvalue );
    size_t size = json_string_length( jvalue );

    *_value = value;
    *_size = size;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_json_dumpb_value( hb_json_handler_t * _handler, const char * _key, char * _buffer, size_t _capacity, size_t * _size )
{
    json_t * jroot = (json_t *)_handler->handler;

    json_t * jvalue = json_object_get( jroot, _key );

    if( jvalue == HB_NULLPTR )
    {
        return 0;
    }

    size_t size = json_dumpb( jvalue, _buffer, _capacity, JSON_COMPACT | JSON_ESCAPE_SLASH );

    *_size = size;

    return 1;
}