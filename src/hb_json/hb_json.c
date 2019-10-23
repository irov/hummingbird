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
hb_result_t hb_json_create( const void * _data, size_t _size, hb_json_handle_t * _handle )
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

        return HB_FAILURE;
    }

    _handle->handle = jroot;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_json_destroy( hb_json_handle_t * _handle )
{
    json_t * jroot = (json_t *)_handle->handle;

    json_decref( jroot );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_string( hb_json_handle_t * _handle, const char * _key, const char ** _value, size_t * _size )
{
    json_t * jroot = (json_t *)_handle->handle;

    json_t * jvalue = json_object_get( jroot, _key );

    if( jvalue == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    const char * value = json_string_value( jvalue );
    size_t size = json_string_length( jvalue );

    *_value = value;
    *_size = size;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_dumpb_value( hb_json_handle_t * _handle, const char * _key, char * _buffer, size_t _capacity, size_t * _size )
{
    json_t * jroot = (json_t *)_handle->handle;

    json_t * jvalue = json_object_get( jroot, _key );

    if( jvalue == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    size_t size = json_dumpb( jvalue, _buffer, _capacity, JSON_COMPACT | JSON_ESCAPE_SLASH );

    *_size = size;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_dumpb( hb_json_handle_t * _handle, char * _buffer, size_t _capacity, size_t * _size )
{
    json_t * jroot = (json_t *)_handle->handle;

    size_t size = json_dumpb( jroot, _buffer, _capacity, JSON_COMPACT | JSON_ESCAPE_SLASH );

    *_size = size;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////