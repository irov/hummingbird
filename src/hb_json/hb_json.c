#include "hb_json.h"

#include "hb_log/hb_log.h"
#include "hb_memory/hb_memory.h"
#include "hb_utils/hb_file.h"
#include "hb_utils/hb_strncpyn.h"

#include "json.h"
#include "json_dump.h"

#include <string.h>
#include <memory.h>

//////////////////////////////////////////////////////////////////////////
static void __js_failed_fun( const char * _pointer, const char * _end, const char * _message, void * _ud )
{
    HB_UNUSED( _pointer );
    HB_UNUSED( _end );
    HB_UNUSED( _ud );

    HB_LOG_MESSAGE_ERROR( "json", "json '%s' error: %s"
        , _pointer
        , _message
    );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_create( const void * _data, hb_size_t _size, void * _pool, hb_size_t _capacity, hb_json_handle_t ** _handle )
{
    js_buffer_t buffer;
    js_make_buffer( _pool, _capacity, &buffer );

    js_allocator_t allocator;
    js_make_allocator_buffer( &buffer, &allocator );

    js_element_t * doc;
    if( js_parse( allocator, js_flag_none, _data, _size, &__js_failed_fun, JS_NULLPTR, &doc ) == JS_FAILURE )
    {
        return HB_FAILURE;
    }

    *_handle = (hb_json_handle_t *)doc;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_mapping( const void * _data, hb_size_t _size, void * _pool, hb_size_t _capacity, hb_json_handle_t ** _handle )
{
    js_buffer_t buffer;
    js_make_buffer( _pool, _capacity, &buffer );

    js_allocator_t allocator;
    js_make_allocator_buffer( &buffer, &allocator );

    js_element_t * doc;
    if( js_parse( allocator, js_flag_string_inplace, _data, _size, &__js_failed_fun, JS_NULLPTR, &doc ) == JS_FAILURE )
    {
        return HB_FAILURE;
    }

    *_handle = (hb_json_handle_t *)doc;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_load( const char * _file, void * _pool, hb_size_t _capacity, hb_json_handle_t ** _handle )
{
    char buffer[HB_DATA_MAX_SIZE];
    hb_size_t buffer_size;
    if( hb_file_read_text( _file, buffer, HB_DATA_MAX_SIZE, &buffer_size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_create( buffer, buffer_size, _pool, _capacity, _handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_json_free( hb_json_handle_t * _handle )
{
    js_element_t * jval = (js_element_t *)_handle;

    js_free( jval );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_update( hb_json_handle_t * _base, hb_json_handle_t * _update, void * _pool, hb_size_t _capacity, hb_json_handle_t ** _result )
{
    js_element_t * jbase = (js_element_t *)_base;
    js_element_t * jupdate = (js_element_t *)_update;

    js_buffer_t buffer;
    js_make_buffer( _pool, _capacity, &buffer );

    js_allocator_t allocator;
    js_make_allocator_buffer( &buffer, &allocator );

    js_element_t * jtotal;
    if( js_patch( allocator, js_flag_none, jbase, jupdate, &jtotal ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    *_result = (hb_json_handle_t *)jtotal;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_dumps( hb_json_handle_t * _handle, char * _buffer, hb_size_t _capacity, hb_size_t * const _size )
{
    js_element_t * jval = (js_element_t *)_handle;

    js_buffer_t dump_buff;
    js_make_buffer( _buffer, _capacity, &dump_buff );

    js_dump_ctx_t dump_ctx;
    js_make_dump_ctx_buffer( &dump_buff, &dump_ctx );

    if( js_dump( jval, &dump_ctx ) == JS_FAILURE )
    {
        return HB_FAILURE;
    }

    js_size_t dump_size = js_get_buffer_size( &dump_buff );

    *_size = (hb_size_t)dump_size;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_json_is_object( const hb_json_handle_t * _handle )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    js_bool_t result = js_is_object( jval );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_json_is_object_empty( const hb_json_handle_t * _handle )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    if( js_is_object( jval ) == HB_FALSE )
    {
        return HB_TRUE;
    }

    if( js_object_size( jval ) == 0 )
    {
        return HB_TRUE;
    }

    return HB_FALSE;
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_json_is_array_empty( const hb_json_handle_t * _handle )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    if( js_is_array( jval ) == HB_FALSE )
    {
        return HB_TRUE;
    }

    if( js_array_size( jval ) == 0 )
    {
        return HB_TRUE;
    }

    return HB_FALSE;
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_json_is_array( const hb_json_handle_t * _handle )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    js_bool_t result = js_is_array( jval );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_size_t hb_json_get_array_size( const hb_json_handle_t * _handle )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    js_size_t size = js_array_size( jval );

    return (hb_size_t)size;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_array_get_element( const hb_json_handle_t * _handle, uint32_t _index, const hb_json_handle_t ** _out )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    const js_element_t * jelement = js_array_get( jval, (js_size_t)_index );

    if( jelement == JS_NULLPTR )
    {
        return HB_FAILURE;
    }

    *_out = (const hb_json_handle_t *)jelement;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field( const hb_json_handle_t * _handle, const char * _key, const hb_json_handle_t ** _out )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    const js_element_t * jelement = js_object_get( jval, _key );

    if( jelement == JS_NULLPTR )
    {
        return HB_FAILURE;
    }

    *_out = (const hb_json_handle_t *)jelement;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_size_t hb_json_get_object_size( const hb_json_handle_t * _handle )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    js_size_t size = js_object_size( jval );

    return (hb_size_t)size;
}
//////////////////////////////////////////////////////////////////////////
hb_json_type_e hb_json_get_type( const hb_json_handle_t * _handle )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    js_type_e t = js_type( jval );

    switch( t )
    {
    case js_type_null:
        return e_hb_json_null;
    case js_type_false:
        return e_hb_json_false;
    case js_type_true:
        return e_hb_json_true;
    case js_type_integer:
        return e_hb_json_integer;
    case js_type_real:
        return e_hb_json_real;
    case js_type_string:
        return e_hb_json_string;
    case js_type_array:
        return e_hb_json_array;
    case js_type_object:
        return e_hb_json_object;
    default:
        return e_hb_json_object;
    }
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_boolean( const hb_json_handle_t * _handle, hb_bool_t * const _value )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    if( js_is_boolean( jval ) == JS_FALSE )
    {
        return HB_FAILURE;
    }

    js_bool_t value = js_get_boolean( jval );

    *_value = (hb_bool_t)value;

    return HB_FAILURE;

}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_int16( const hb_json_handle_t * _handle, int16_t * const _value )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    if( js_is_integer( jval ) == JS_TRUE )
    {
        js_integer_t value = js_get_integer( jval );

        *_value = (int16_t)value;

        return HB_SUCCESSFUL;
    }
    else if( js_is_real( jval ) == JS_TRUE )
    {
        js_real_t value = js_get_real( jval );

        *_value = (int16_t)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_int32( const hb_json_handle_t * _handle, int32_t * const _value )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    if( js_is_integer( jval ) == JS_TRUE )
    {
        js_integer_t value = js_get_integer( jval );

        *_value = (int32_t)value;

        return HB_SUCCESSFUL;
    }
    else if( js_is_real( jval ) == JS_TRUE )
    {
        js_real_t value = js_get_real( jval );

        *_value = (int32_t)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_uint16( const hb_json_handle_t * _handle, uint16_t * const _value )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    if( js_is_integer( jval ) == JS_TRUE )
    {
        js_integer_t value = js_get_integer( jval );

        *_value = (uint16_t)value;

        return HB_SUCCESSFUL;
    }
    else if( js_is_real( jval ) == JS_TRUE )
    {
        js_real_t value = js_get_real( jval );

        *_value = (uint16_t)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_uint32( const hb_json_handle_t * _handle, uint32_t * const _value )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    if( js_is_integer( jval ) == JS_TRUE )
    {
        js_integer_t value = js_get_integer( jval );

        *_value = (uint32_t)value;

        return HB_SUCCESSFUL;
    }
    else if( js_is_real( jval ) == JS_TRUE )
    {
        js_real_t value = js_get_real( jval );

        *_value = (uint32_t)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_int64( const hb_json_handle_t * _handle, int64_t * const _value )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    if( js_is_integer( jval ) == JS_TRUE )
    {
        js_integer_t value = js_get_integer( jval );

        *_value = (int64_t)value;

        return HB_SUCCESSFUL;
    }
    else if( js_is_real( jval ) == JS_TRUE )
    {
        js_real_t value = js_get_real( jval );

        *_value = (int64_t)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_uint64( const hb_json_handle_t * _handle, uint64_t * const _value )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    if( js_is_integer( jval ) == JS_TRUE )
    {
        js_integer_t value = js_get_integer( jval );

        *_value = (uint64_t)value;

        return HB_SUCCESSFUL;
    }
    else if( js_is_real( jval ) == JS_TRUE )
    {
        js_real_t value = js_get_real( jval );

        *_value = (uint64_t)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_size_t( const hb_json_handle_t * _handle, hb_size_t * const _value )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    if( js_is_integer( jval ) == JS_TRUE )
    {
        js_integer_t value = js_get_integer( jval );

        *_value = (hb_size_t)value;

        return HB_SUCCESSFUL;
    }
    else if( js_is_real( jval ) == JS_TRUE )
    {
        js_real_t value = js_get_real( jval );

        *_value = (hb_size_t)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_float( const hb_json_handle_t * _handle, float * const _value )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    if( js_is_real( jval ) == JS_TRUE )
    {
        js_real_t value = js_get_real( jval );

        *_value = (float)value;

        return HB_SUCCESSFUL;
    }
    else if( js_is_integer( jval ) == JS_TRUE )
    {
        js_integer_t value = js_get_integer( jval );

        *_value = (float)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_double( const hb_json_handle_t * _handle, double * const _value )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    if( js_is_real( jval ) == JS_TRUE )
    {
        js_real_t value = js_get_real( jval );

        *_value = (double)value;

        return HB_SUCCESSFUL;
    }
    else if( js_is_integer( jval ) == JS_TRUE )
    {
        js_integer_t value = js_get_integer( jval );

        *_value = (double)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_string( const hb_json_handle_t * _handle, hb_json_string_t * const _value )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    if( js_is_string( jval ) == JS_FALSE )
    {
        return HB_FAILURE;
    }

    js_string_t value;
    js_get_string( jval, &value );

    _value->value = value.value;
    _value->size = (hb_size_t)value.size;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_json_is_string( const hb_json_handle_t * _handle )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    if( js_is_string( jval ) == JS_FALSE )
    {
        return HB_FALSE;
    }

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_copy_string( const hb_json_handle_t * _handle, char * _value, hb_size_t _capacity, hb_size_t * const _size )
{
    const js_element_t * jval = (const js_element_t *)_handle;

    if( js_is_string( jval ) == JS_FALSE )
    {
        return HB_FAILURE;
    }

    js_string_t value;
    js_get_string( jval, &value );

    if( hb_strncpyn( _value, _capacity, value.value, value.size ) == HB_FALSE )
    {
        return HB_FAILURE;
    }

    if( _size != HB_NULLPTR )
    {
        *_size = value.size;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_string( const hb_json_handle_t * _handle, const char * _key, hb_json_string_t * const _value )
{
    const hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_to_string( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_json_get_field_string_default( const hb_json_handle_t * _handle, const char * _key, hb_json_string_t * const _value, const char * _default )
{
    if( hb_json_get_field_string( _handle, _key, _value ) == HB_SUCCESSFUL )
    {
        return;
    }

    _value->value = _default;

    if( _default != HB_NULLPTR )
    {
        _value->size = strlen( _default );
    }
    else
    {
        _value->size = 0;
    }
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_copy_field_string( const hb_json_handle_t * _handle, const char * _key, char * _value, hb_size_t _capacity )
{
    const hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_string_t json_string;
    if( hb_json_to_string( field, &json_string ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_strncpyn( _value, _capacity, json_string.value, json_string.size ) == HB_FALSE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_copy_field_string_default( const hb_json_handle_t * _handle, const char * _key, char * _value, hb_size_t _capacity, const char * _default )
{
    if( hb_json_copy_field_string( _handle, _key, _value, _capacity ) == HB_SUCCESSFUL )
    {
        return HB_SUCCESSFUL;
    }

    hb_size_t default_len = strlen( _default );

    if( hb_strncpyn( _value, _capacity, _default, default_len ) == HB_FALSE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_boolean( const hb_json_handle_t * _handle, const char * _key, hb_bool_t * const _value )
{
    const hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_to_boolean( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;

}
//////////////////////////////////////////////////////////////////////////
void hb_json_get_field_boolean_default( const hb_json_handle_t * _handle, const char * _key, hb_bool_t * const _value, hb_bool_t _default )
{
    if( hb_json_get_field_boolean( _handle, _key, _value ) == HB_SUCCESSFUL )
    {
        return;
    }

    *_value = _default;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_int16( const hb_json_handle_t * _handle, const char * _key, int16_t * _value )
{
    const hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_to_int16( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_json_get_field_int16_default( const hb_json_handle_t * _handle, const char * _key, int16_t * _value, int16_t _default )
{
    const hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_SUCCESSFUL )
    {
        return;
    }

    *_value = _default;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_int32( const hb_json_handle_t * _handle, const char * _key, int32_t * _value )
{
    const hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_to_int32( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_json_get_field_int32_default( const hb_json_handle_t * _handle, const char * _key, int32_t * _value, int32_t _default )
{
    if( hb_json_get_field_int32( _handle, _key, _value ) == HB_SUCCESSFUL )
    {
        return;
    }

    *_value = _default;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_int64( const hb_json_handle_t * _handle, const char * _key, int64_t * _value )
{
    const hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_to_int64( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_json_get_field_int64_default( const hb_json_handle_t * _handle, const char * _key, int64_t * _value, int64_t _default )
{
    if( hb_json_get_field_int64( _handle, _key, _value ) == HB_SUCCESSFUL )
    {
        return;
    }

    *_value = _default;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_uint16( const hb_json_handle_t * _handle, const char * _key, uint16_t * _value )
{
    const hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_to_uint16( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_json_get_field_uint16_default( const hb_json_handle_t * _handle, const char * _key, uint16_t * _value, uint16_t _default )
{
    if( hb_json_get_field_uint16( _handle, _key, _value ) == HB_SUCCESSFUL )
    {
        return;
    }

    *_value = _default;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_uint32( const hb_json_handle_t * _handle, const char * _key, uint32_t * const _value )
{
    const hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_to_uint32( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_json_get_field_uint32_default( const hb_json_handle_t * _handle, const char * _key, uint32_t * const _value, uint32_t _default )
{
    if( hb_json_get_field_uint32( _handle, _key, _value ) == HB_SUCCESSFUL )
    {
        return;
    }

    *_value = _default;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_uint64( const hb_json_handle_t * _handle, const char * _key, uint64_t * _value )
{
    const hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_to_uint64( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_json_get_field_uint64_default( const hb_json_handle_t * _handle, const char * _key, uint64_t * _value, uint64_t _default )
{
    if( hb_json_get_field_uint64( _handle, _key, _value ) == HB_SUCCESSFUL )
    {
        return;
    }

    *_value = _default;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_size_t( const hb_json_handle_t * _handle, const char * _key, hb_size_t * const _value )
{
    const hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_to_size_t( field, _value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_json_get_field_size_t_default( const hb_json_handle_t * _handle, const char * _key, hb_size_t * const _value, hb_size_t _default )
{
    if( hb_json_get_field_size_t( _handle, _key, _value ) == HB_SUCCESSFUL )
    {
        return;
    }

    *_value = _default;
}
//////////////////////////////////////////////////////////////////////////
typedef struct __js_object_foreach_data_t
{
    hb_json_object_visit_t visitor;
    void * ud;
} __js_object_foreach_data_t;
//////////////////////////////////////////////////////////////////////////
static js_result_t __json_visitor_object( js_size_t _index, const js_element_t * _key, const js_element_t * _value, void * _ud )
{
    __js_object_foreach_data_t * data = (__js_object_foreach_data_t *)_ud;

    const hb_json_handle_t * key = (const hb_json_handle_t *)_key;
    const hb_json_handle_t * value = (const hb_json_handle_t *)_value;

    if( (*data->visitor)((hb_size_t)_index, key, value, data->ud) == HB_FAILURE )
    {
        return JS_FAILURE;
    }

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_visit_object( const hb_json_handle_t * _handle, hb_json_object_visit_t _visitor, void * _ud )
{
    js_element_t * jval = (js_element_t *)_handle;

    __js_object_foreach_data_t data;
    data.visitor = _visitor;
    data.ud = _ud;

    if( js_object_visit( jval, &__json_visitor_object, &data ) == JS_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
typedef struct __js_array_foreach_data_t
{
    hb_json_array_visit_t visitor;
    void * ud;
} __js_array_foreach_data_t;
//////////////////////////////////////////////////////////////////////////
static js_result_t __json_visit_array( js_size_t _index, const js_element_t * _value, void * _ud )
{
    __js_array_foreach_data_t * data = (__js_array_foreach_data_t *)_ud;

    const hb_json_handle_t * value = (const hb_json_handle_t *)_value;

    if( (*data->visitor)((hb_size_t)_index, value, data->ud) == HB_FAILURE )
    {
        return JS_FAILURE;
    }

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_visit_array( const hb_json_handle_t * _handle, hb_json_array_visit_t _visitor, void * _ud )
{
    js_element_t * jval = (js_element_t *)_handle;

    __js_array_foreach_data_t data;
    data.visitor = _visitor;
    data.ud = _ud;

    if( js_array_visit( jval, &__json_visit_array, &data ) == JS_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////