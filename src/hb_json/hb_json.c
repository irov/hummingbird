#include "hb_json.h"

#include "hb_log/hb_log.h"
#include "hb_memory/hb_memory.h"
#include "hb_utils/hb_file.h"

#include "json.h"

#include <string.h>
#include <memory.h>

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_create( const void * _data, hb_size_t _size, void * _pool, hb_size_t _capacity, hb_json_handle_t ** _handle )
{
    yyjson_alc alc;
    yyjson_alc_pool_init( &alc, _pool, _capacity );

    yyjson_read_err err;
    yyjson_doc * doc = yyjson_read_opts( (char *)_data, _size, YYJSON_READ_NOFLAG, &alc, &err );

    if( doc == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_ERROR( "json", "json '%.*s' error pos: %zu code: %u message: %s"
            , _size
            , (const char *)_data
            , err.pos
            , err.code
            , err.msg
        );

        return HB_FAILURE;
    }

    yyjson_val * root = yyjson_doc_get_root( doc );

    *_handle = (hb_json_handle_t *)root;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_mapping( void * _data, hb_size_t _size, void * _pool, hb_size_t _capacity, hb_json_handle_t ** _handle )
{
    yyjson_alc alc;
    yyjson_alc_pool_init( &alc, _pool, _capacity );

    yyjson_read_err err;
    yyjson_doc * doc = yyjson_read_opts( (char *)_data, _size, YYJSON_READ_INSITU, &alc, &err );

    if( doc == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_ERROR( "json", "json '%.*s' error pos: %zu code: %u message: %s"
            , _size
            , (const char *)_data
            , err.pos
            , err.code
            , err.msg
        );

        return HB_FAILURE;
    }

    yyjson_val * root = yyjson_doc_get_root( doc );

    *_handle = (hb_json_handle_t *)root;

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
hb_result_t hb_json_update( hb_json_handle_t * _base, hb_json_handle_t * _update, void * _pool, hb_size_t _capacity, hb_json_handle_t ** _result )
{
    yyjson_val * jbase = (yyjson_val *)_base;
    yyjson_val * jupdate = (yyjson_val *)_update;

    yyjson_alc alc;
    yyjson_alc_pool_init( &alc, _pool, _capacity );

    yyjson_mut_doc * jdoc = yyjson_mut_doc_new( &alc );
    
    yyjson_mut_val * jpatch = yyjson_merge_patch( jdoc, jbase, jupdate );

    if( jpatch == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    * _result = (hb_json_handle_t *)jpatch;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_dumps( hb_json_handle_t * _handle, char * _buffer, hb_size_t _capacity, hb_size_t * _size )
{
    yyjson_val * jval = (yyjson_val *)_handle;

    size_t len;
    char * str = yyjson_val_write( jval, YYJSON_WRITE_NOFLAG, &len );

    if( str == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    strncpy( _buffer, str, _capacity );
    _buffer[_capacity - 1] = '\0';

    *_size = len;

    free( str );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_json_is_object_empty( const hb_json_handle_t * _handle )
{
    yyjson_val * jval = (yyjson_val *)_handle;

    if( yyjson_is_obj( jval ) == HB_FALSE )
    {
        return HB_TRUE;
    }

    if( yyjson_obj_size( jval ) == 0 )
    {
        return HB_TRUE;
    }

    return HB_FALSE;
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_json_is_array_empty( const hb_json_handle_t * _handle )
{
    yyjson_val * jval = (yyjson_val *)_handle;

    if( yyjson_is_arr( jval ) == HB_FALSE )
    {
        return HB_FALSE;
    }

    if( yyjson_arr_size( jval ) != 0 )
    {
        return HB_FALSE;
    }

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_json_is_array( const hb_json_handle_t * _handle )
{
    yyjson_val * jval = (yyjson_val *)_handle;

    bool result = yyjson_is_arr( jval );

    return result;
}
//////////////////////////////////////////////////////////////////////////
uint32_t hb_json_array_count( const hb_json_handle_t * _handle )
{
    yyjson_val * jval = (yyjson_val *)_handle;

    size_t size = yyjson_arr_size( jval );

    return (uint32_t)size;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_array_get( const hb_json_handle_t * _handle, uint32_t _index, hb_json_handle_t ** _out )
{
    yyjson_val * jval = (yyjson_val *)_handle;

    yyjson_val * jelement = yyjson_arr_get( jval, (hb_size_t)_index );

    if( jelement == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    *_out = (hb_json_handle_t *)jelement;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field( const hb_json_handle_t * _handle, const char * _key, hb_json_handle_t ** _out )
{
    yyjson_val * jval = (yyjson_val *)_handle;

    yyjson_val * jelement = yyjson_obj_get( jval, _key );

    if( jelement == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    *_out = (hb_json_handle_t *)jelement;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
uint32_t hb_json_get_fields_count( const hb_json_handle_t * _handle )
{
    yyjson_val * jval = (yyjson_val *)_handle;

    size_t size = yyjson_obj_size( jval );

    return (uint32_t)size;
}
//////////////////////////////////////////////////////////////////////////
hb_json_type_t hb_json_get_type( const hb_json_handle_t * _handle )
{
    yyjson_val * val = (yyjson_val *)_handle;

    yyjson_type t = yyjson_get_type( val );

    switch( t )
    {
    case YYJSON_TYPE_OBJ:
        return e_hb_json_object;
    case YYJSON_TYPE_ARR:
        return e_hb_json_array;
    case YYJSON_TYPE_STR:
        return e_hb_json_string;
    case YYJSON_TYPE_NUM:
        {
            yyjson_subtype subtype = yyjson_get_subtype( val );

            switch( subtype )
            {
            case YYJSON_SUBTYPE_UINT:
                return e_hb_json_integer;
            case YYJSON_SUBTYPE_SINT:
                return e_hb_json_integer;
            case YYJSON_SUBTYPE_REAL:
                return e_hb_json_real;
            default:
                return e_hb_json_invalid;
            }
        }
    case YYJSON_TYPE_BOOL:
        {
            yyjson_subtype subtype = yyjson_get_subtype( val );

            switch( subtype )
            {
            case YYJSON_SUBTYPE_FALSE:
                return e_hb_json_false;
            case YYJSON_SUBTYPE_TRUE:
                return e_hb_json_true;
            default:
                return e_hb_json_invalid;
            }
        }break;
    case YYJSON_TYPE_NULL:
        return e_hb_json_null;
    default:
        return e_hb_json_invalid;
    }
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_string( const hb_json_handle_t * _handle, const char ** _value, hb_size_t * _size )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_str( val ) == false )
    {
        return HB_FAILURE;
    }

    const char * value = unsafe_yyjson_get_str( val );
    hb_size_t size = unsafe_yyjson_get_len( val );

    *_value = value;

    if( _size != HB_NULLPTR )
    {
        *_size = size;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_boolean( const hb_json_handle_t * _handle, hb_bool_t * const _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_bool( val ) == false )
    {
        return HB_FAILURE;
    }

    bool value = unsafe_yyjson_get_bool( val );

    *_value = (hb_bool_t)value;

    return HB_FAILURE;

}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_int16( const hb_json_handle_t * _handle, int16_t * _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (int16_t)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (int16_t)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (int16_t)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_int32( const hb_json_handle_t * _handle, int32_t * _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (int32_t)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (int32_t)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (int32_t)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_uint16( const hb_json_handle_t * _handle, uint16_t * _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (uint16_t)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (uint16_t)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (uint16_t)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_uint32( const hb_json_handle_t * _handle, uint32_t * _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (uint32_t)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (uint32_t)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (uint32_t)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_int64( const hb_json_handle_t * _handle, int64_t * _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (int64_t)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (int64_t)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (int64_t)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_uint64( const hb_json_handle_t * _handle, uint64_t * _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (uint64_t)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (uint64_t)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (uint64_t)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_float( const hb_json_handle_t * _handle, float * const _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (float)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (float)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (float)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_to_double( const hb_json_handle_t * _handle, double * const _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (double)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (double)value;

        return HB_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (double)value;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_string( hb_json_handle_t * _handle, const char * _key, const char ** _value, hb_size_t * const _size )
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

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_string_default( hb_json_handle_t * _handle, const char * _key, const char ** _value, hb_size_t * _size, const char * _default )
{
    if( hb_json_get_field_string( _handle, _key, _value, _size ) == HB_SUCCESSFUL )
    {
        return HB_SUCCESSFUL;
    }

    if( _default == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    *_value = _default;

    if( _size != HB_NULLPTR )
    {
        *_size = strlen( _default );
    }

    return HB_SUCCESSFUL;    
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_copy_field_string( hb_json_handle_t * _handle, const char * _key, char * _value, hb_size_t _capacity )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    const char * value;
    hb_size_t size;
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

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_copy_field_string_default( hb_json_handle_t * _handle, const char * _key, char * _value, hb_size_t _capacity, const char * _default )
{
    if( hb_json_copy_field_string( _handle, _key, _value, _capacity ) == HB_SUCCESSFUL )
    {
        return HB_SUCCESSFUL;
    }

    hb_size_t default_len = strlen( _default );

    if( default_len > _capacity )
    {
        return HB_FAILURE;
    }

    memcpy( _value, _default, default_len );
    _value[default_len] = '\0';

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_int16( hb_json_handle_t * _handle, const char * _key, int16_t * _value )
{
    hb_json_handle_t * field;
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
hb_result_t hb_json_get_field_int16_default( hb_json_handle_t * _handle, const char * _key, int16_t * _value, int16_t _default )
{
    hb_json_handle_t * field;
    if( hb_json_get_field( _handle, _key, &field ) == HB_SUCCESSFUL )
    {
        return HB_SUCCESSFUL;
    }

    *_value = _default;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_int32( hb_json_handle_t * _handle, const char * _key, int32_t * _value )
{
    hb_json_handle_t * field;
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
hb_result_t hb_json_get_field_int32_default( hb_json_handle_t * _handle, const char * _key, int32_t * _value, int32_t _default )
{
    if( hb_json_get_field_int32( _handle, _key, _value ) == HB_SUCCESSFUL )
    {
        return HB_SUCCESSFUL;
    }

    *_value = _default;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_int64( hb_json_handle_t * _handle, const char * _key, int64_t * _value )
{
    hb_json_handle_t * field;
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
hb_result_t hb_json_get_field_int64_default( hb_json_handle_t * _handle, const char * _key, int64_t * _value, int64_t _default )
{
    if( hb_json_get_field_int64( _handle, _key, _value ) == HB_SUCCESSFUL )
    {
        return HB_SUCCESSFUL;
    }

    *_value = _default;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_uint16( hb_json_handle_t * _handle, const char * _key, uint16_t * _value )
{
    hb_json_handle_t * field;
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
hb_result_t hb_json_get_field_uint16_default( hb_json_handle_t * _handle, const char * _key, uint16_t * _value, uint16_t _default )
{
    if( hb_json_get_field_uint16( _handle, _key, _value ) == HB_SUCCESSFUL )
    {
        return HB_SUCCESSFUL;
    }

    *_value = _default;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_uint32( hb_json_handle_t * _handle, const char * _key, uint32_t * const _value )
{
    hb_json_handle_t * field;
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
hb_result_t hb_json_get_field_uint32_default( hb_json_handle_t * _handle, const char * _key, uint32_t * const _value, uint32_t _default )
{
    if( hb_json_get_field_uint32( _handle, _key, _value ) == HB_SUCCESSFUL )
    {
        return HB_SUCCESSFUL;
    }

    *_value = _default;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_get_field_uint64( hb_json_handle_t * _handle, const char * _key, uint64_t * _value )
{
    hb_json_handle_t * field;
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
hb_result_t hb_json_get_field_uint64_default( hb_json_handle_t * _handle, const char * _key, uint64_t * _value, uint64_t _default )
{
    if( hb_json_get_field_uint64( _handle, _key, _value ) == HB_SUCCESSFUL )
    {
        return HB_SUCCESSFUL;
    }

    *_value = _default;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_object_foreach( const hb_json_handle_t * _handle, hb_json_object_visitor_t _visitor, void * _ud )
{
    yyjson_val * obj = (yyjson_val *)_handle;

    size_t idx, max;
    yyjson_val * objkey;
    yyjson_val * objval;
    yyjson_obj_foreach( obj, idx, max, objkey, objval )
    {
        const hb_json_handle_t * objkeyhandle = (const hb_json_handle_t *)objkey;
        const hb_json_handle_t * objvalhandle = (const hb_json_handle_t *)objval;

        if( (*_visitor)((hb_size_t)idx, objkeyhandle, objvalhandle, _ud) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_json_array_foreach( const hb_json_handle_t * _handle, hb_json_array_visitor_t _visitor, void * _ud )
{
    yyjson_val * arr = (yyjson_val *)_handle;

    size_t idx, max;
    yyjson_val * val;
    yyjson_arr_foreach( arr, idx, max, val )
    {
        const hb_json_handle_t * idhandle = (const hb_json_handle_t *)val;

        if( (*_visitor)((hb_size_t)idx, idhandle, _ud) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////