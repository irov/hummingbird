#include "hb_json.h"

#include "hb_log/hb_log.h"
#include "hb_memory/hb_memory.h"
#include "hb_utils/hb_file.h"

#include "jansson.h"

#include <string.h>
#include <memory.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_json_handle_t
{
    json_t * jroot;
}hb_json_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_json_load_data_t
{
    const hb_byte_t * buffer;
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

    const hb_byte_t * jd_buffer = jd->buffer + jd->carriage;
    memcpy( _buffer, jd_buffer, _buflen );
    jd->carriage += _buflen;

    return _buflen;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_create( const void * _data, size_t _size, hb_json_handle_t ** _handle )
{
    hb_json_load_data_t jd;
    jd.buffer = (const hb_byte_t *)(_data);
    jd.carriage = 0;
    jd.capacity = _size;

    json_error_t er;
    json_t * jroot = json_load_callback( &__hb_json_load_callback, &jd, 0, &er );

    if( jroot == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_ERROR( "json", "json '%s' error line [%d] column [%d] position [%d]: %s"
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

    HB_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_load( const char * _file, hb_json_handle_t ** _handle )
{
    char buffer[HB_DATA_MAX_SIZE];
    size_t buffer_size;
    if( hb_file_read( _file, buffer, HB_DATA_MAX_SIZE, &buffer_size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_create( buffer, buffer_size, _handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_update( hb_json_handle_t * _base, hb_json_handle_t * _update )
{
    json_t * jbase = _base->jroot;
    json_t * jupdate = _update->jroot;

    if( json_object_update( jbase, jupdate ) == -1 )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_dumps( hb_json_handle_t * _handle, char * _buffer, size_t _capacity, size_t * _size )
{
    json_t * jroot = _handle->jroot;

    size_t sz = json_dumpb( jroot, _buffer, _capacity, JSON_COMPACT );

    if( sz == 0 )
    {
        return HB_FAILURE;
    }

    *_size = sz;

    return HB_SUCCESSFUL;
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

    json_incref( jvalue );

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
hb_result_t hb_json_to_int16( hb_json_handle_t * _handle, int16_t * _value )
{
    json_t * jvalue = _handle->jroot;

    json_int_t value = json_integer_value( jvalue );

    *_value = (int16_t)value;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_int32( hb_json_handle_t * _handle, int32_t * _value )
{
    json_t * jvalue = _handle->jroot;

    json_int_t value = json_integer_value( jvalue );

    *_value = (int32_t)value;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_uint16( hb_json_handle_t * _handle, uint16_t * _value )
{
    json_t * jvalue = _handle->jroot;

    json_int_t value = json_integer_value( jvalue );

    *_value = (uint16_t)value;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_uint32( hb_json_handle_t * _handle, uint32_t * _value )
{
    json_t * jvalue = _handle->jroot;

    json_int_t value = json_integer_value( jvalue );

    *_value = (uint32_t)value;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_int64( hb_json_handle_t * _handle, int64_t * _value )
{
    json_t * jvalue = _handle->jroot;

    json_int_t value = json_integer_value( jvalue );

    *_value = (int64_t)value;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_uint64( hb_json_handle_t * _handle, uint64_t * _value )
{
    json_t * jvalue = _handle->jroot;

    json_int_t value = json_integer_value( jvalue );

    *_value = (uint64_t)value;

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
hb_result_t hb_json_get_field_string( hb_json_handle_t * _handle, const char * _key, const char ** _value, size_t * _size, const char * _default )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        if( _default != HB_NULLPTR )
        {
            *_value = _default;

            return HB_SUCCESSFUL;
        }
        else
        {
            return HB_FAILURE;
        }
    }

    if( hb_json_to_string( field, _value, _size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_destroy( field );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_copy_field_string( hb_json_handle_t * _handle, const char * _key, char * _value, size_t _capacity, const char * _default )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        size_t default_len = strlen( _default );

        if( default_len > _capacity )
        {
            return HB_FAILURE;
        }        

        memcpy( _value, _default, default_len );
        _value[default_len] = '\0';

        return HB_SUCCESSFUL;
    }

    const char * value;
    size_t size;
    if( hb_json_to_string( field, &value, &size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( size > _capacity )
    {
        return HB_FAILURE;
    }

    memcpy( _value, value, size );
    _value[size] = '\0';

    hb_json_destroy( field );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_copy_field_string_required( hb_json_handle_t * _handle, const char * _key, char * _value, size_t _capacity, hb_bool_t * _result )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        *_result = HB_FALSE;

        return HB_SUCCESSFUL;
    }

    const char * value;
    size_t size;
    if( hb_json_to_string( field, &value, &size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( size > _capacity )
    {
        return HB_FAILURE;
    }

    memcpy( _value, value, size );
    _value[size] = '\0';

    hb_json_destroy( field );

    //it's feature for requireds flags!
    //*_result = HB_TRUE;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_int16( hb_json_handle_t * _handle, const char * _key, int16_t * _value, int16_t _default )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        *_value = _default;

        return HB_SUCCESSFUL;
    }

    if( hb_json_to_int16( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_destroy( field );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_int32( hb_json_handle_t * _handle, const char * _key, int32_t * _value, int32_t _default )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        *_value = _default;

        return HB_SUCCESSFUL;
    }

    if( hb_json_to_int32( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_destroy( field );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_int64( hb_json_handle_t * _handle, const char * _key, int64_t * _value, int64_t _default )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        *_value = _default;

        return HB_SUCCESSFUL;
    }

    if( hb_json_to_int64( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_destroy( field );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_uint16( hb_json_handle_t * _handle, const char * _key, uint16_t * _value, uint16_t _default )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        *_value = _default;

        return HB_SUCCESSFUL;
    }

    if( hb_json_to_uint16( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_destroy( field );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_uint32( hb_json_handle_t * _handle, const char * _key, uint32_t * _value, uint32_t _default )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        *_value = _default;

        return HB_SUCCESSFUL;
    }

    if( hb_json_to_uint32( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_destroy( field );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_uint64( hb_json_handle_t * _handle, const char * _key, uint64_t * _value, uint64_t _default )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        *_value = _default;

        return HB_SUCCESSFUL;
    }

    if( hb_json_to_uint64( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_destroy( field );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_int32_required( hb_json_handle_t * _handle, const char * _key, int32_t * _value, hb_bool_t * _result )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        *_result = HB_FALSE;

        return HB_SUCCESSFUL;
    }

    if( hb_json_to_int32( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_destroy( field );

    //it's feature for requireds flags!
    //*_result = HB_TRUE;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_uint32_required( hb_json_handle_t * _handle, const char * _key, uint32_t * _value, hb_bool_t * _result )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        *_result = HB_FALSE;

        return HB_SUCCESSFUL;
    }

    if( hb_json_to_uint32( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_destroy( field );

    //it's feature for requireds flags!
    //*_result = HB_TRUE;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_int64_required( hb_json_handle_t * _handle, const char * _key, int64_t * _value, hb_bool_t * _result )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        *_result = HB_FALSE;

        return HB_SUCCESSFUL;
    }

    if( hb_json_to_int64( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_destroy( field );

    //it's feature for requreds flags!
    //*_result = HB_TRUE;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_uint64_required( hb_json_handle_t * _handle, const char * _key, uint64_t * _value, hb_bool_t * _result )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        *_result = HB_FALSE;

        return HB_SUCCESSFUL;
    }

    if( hb_json_to_uint64( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_destroy( field );

    //it's feature for requreds flags!
    //*_result = HB_TRUE;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_foreach( hb_json_handle_t * _handle, hb_json_visitor_t _visitor, void * _ud )
{
    json_t * jvalue = _handle->jroot;

    for( const char * key = json_object_iter_key( json_object_iter( jvalue ) );
        key != NULL;
        key = json_object_iter_key( json_object_iter_next( jvalue, json_object_key_to_iter( key ) ) ) )
    {
        json_t * jelement = json_object_iter_value( json_object_key_to_iter( key ) );

        hb_json_handle_t handle;
        handle.jroot = jelement;
        if( (*_visitor)(key, &handle, _ud) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }

    return HB_SUCCESSFUL;
}