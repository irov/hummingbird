#include "hb_json.h"

#include "hb_log/hb_log.h"

#include "jansson.h"

#include <memory.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_json_handle_t
{
    json_t * jroot;
}hb_json_handle_t;
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
hb_result_t hb_json_create( const void * _data, size_t _size, hb_json_handle_t ** _handle )
{
    hb_json_load_data_t jd;
    jd.buffer = (const uint8_t *)(_data);
    jd.carriage = 0;
    jd.capacity = _size;

    json_error_t er;
    json_t * jroot = json_load_callback( &__hb_json_load_callback, &jd, 0, &er );

    if( jroot == HB_NULLPTR )
    {
        hb_log_message( "json", HB_LOG_ERROR, "json '%s' error line [%d] column [%d] position [%d]: %s"
            , er.source
            , er.line
            , er.column
            , er.position
            , er.text
        );

        return HB_FAILURE;
    }

    hb_json_handle_t * handle = HB_NEW( hb_json_handle_t );

    handle->jroot = jroot;

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_json_destroy( hb_json_handle_t * _handle )
{
    json_t * jroot = _handle->jroot;

    json_decref( jroot );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field( hb_json_handle_t * _handle, const char * _key, hb_json_handle_t ** _out )
{
    json_t * jroot = _handle->jroot;

    json_t * jvalue = json_object_get( jroot, _key );

    if( jvalue == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    hb_json_handle_t * handle = HB_NEW( hb_json_handle_t );
    handle->jroot = jvalue;

    *_out = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
uint32_t hb_json_get_fields_count( hb_json_handle_t * _handle )
{
    json_t * jroot = _handle->jroot;

    size_t jcount = json_object_size( jroot );

    return (uint32_t)jcount;
}
//////////////////////////////////////////////////////////////////////////
hb_json_type_t hb_json_get_type( hb_json_handle_t * _handle )
{
    json_t * jvalue = _handle->jroot;

    json_type jtype = json_typeof( jvalue );

    switch( jtype )
    {
    case JSON_OBJECT:
        return e_hb_json_object;
    case JSON_ARRAY:
        return e_hb_json_array;
    case JSON_STRING:
        return e_hb_json_string;
    case JSON_INTEGER:
        return e_hb_json_integer;
    case JSON_REAL:
        return e_hb_json_real;
    case JSON_TRUE:
        return e_hb_json_true;
    case JSON_FALSE:
        return e_hb_json_false;
    case JSON_NULL:
        return e_hb_json_null;
    default:
        return 0;
    }
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_string( hb_json_handle_t * _handle, const char ** _value, size_t * _size )
{
    json_t * jvalue = _handle->jroot;

    const char * value = json_string_value( jvalue );
    size_t size = json_string_length( jvalue );

    *_value = value;

    if( _size != HB_NULLPTR )
    {
        *_size = size;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_integer( hb_json_handle_t * _handle, int64_t * _value )
{
    json_t * jvalue = _handle->jroot;

    json_int_t value = json_integer_value( jvalue );

    *_value = (int64_t)value;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_real( hb_json_handle_t * _handle, double * _value )
{
    json_t * jvalue = _handle->jroot;

    double value = json_real_value( jvalue );

    *_value = (double)value;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_string( hb_json_handle_t * _handle, const char * _key, const char ** _value, size_t * _size )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_to_string( field, _value, _size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_destroy( field );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_json_foreach( hb_json_handle_t * _handle, hb_json_visitor_t _visitor, void * _ud )
{
    json_t * jvalue = _handle->jroot;

    for( const char * key = json_object_iter_key( json_object_iter( jvalue ) ); key != NULL; key = json_object_iter_key( json_object_iter_next( jvalue, json_object_key_to_iter( key ) ) ) )
    {
        json_t * jelement = json_object_iter_value( json_object_key_to_iter( key ) );

        hb_json_handle_t handle;
        handle.jroot = jelement;
        (*_visitor)(key, &handle, _ud);
    }
}