#include "hb_script_json.h"

#include <stdio.h>

#ifndef HB_SCRIPT_USER_PUBLIC_DATA_FIELD_MAX
#define HB_SCRIPT_USER_PUBLIC_DATA_FIELD_MAX 16
#endif

//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_script_json_value_dumps( lua_State * L, int _index, char * _buffer, size_t * _offset, size_t _capacity )
{
    HB_UNUSED( _capacity );

    int type = lua_type( L, _index );

    switch( type )
    {
    case LUA_TNIL:
        {
            *_offset += sprintf( _buffer + *_offset, "null" );
        }break;
    case LUA_TBOOLEAN:
        {
            int result = lua_toboolean( L, _index );

            *_offset += sprintf( _buffer + *_offset, result == 1 ? "true" : "false" );
        }break;
    case LUA_TNUMBER:
        {
            if( lua_isinteger( L, _index ) == 0 )
            {
                lua_Number result = lua_tonumber( L, _index );

                *_offset += sprintf( _buffer + *_offset, LUA_NUMBER_FMT, result );
            }
            else
            {
                lua_Integer result = lua_tointeger( L, _index );

                *_offset += sprintf( _buffer + *_offset, LUA_INTEGER_FMT, result );
            }
        }break;
    case LUA_TSTRING:
        {
            const char * result = lua_tostring( L, _index );

            *_offset += sprintf( _buffer + *_offset, "\"%s\"", result );
        }break;
    case LUA_TTABLE:
        {
            lua_Unsigned len = lua_rawlen( L, _index );

            if( len == 0 )
            {
                *_offset += sprintf( _buffer + *_offset, "{" );

                lua_pushvalue( L, _index );
                lua_pushnil( L );
                int it = lua_next( L, -2 );
                while( it != 0 )
                {
                    const char * key = lua_tostring( L, -2 );
                    *_offset += sprintf( _buffer + *_offset, "\"%s\": ", key );

                    if( __hb_script_json_value_dumps( L, -1, _buffer, _offset, _capacity ) == HB_FAILURE )
                    {
                        return HB_FAILURE;
                    }

                    lua_pop( L, 1 );

                    it = lua_next( L, -2 );

                    if( it == 0 )
                    {
                        break;
                    }

                    *_offset += sprintf( _buffer + *_offset, ", " );
                }

                lua_pop( L, 1 );

                *_offset += sprintf( _buffer + *_offset, "}" );
            }
            else
            {
                *_offset += sprintf( _buffer + *_offset, "[" );

                lua_pushvalue( L, _index );
                lua_pushnil( L );
                int it = lua_next( L, -2 );
                while( it != 0 )
                {
                    if( __hb_script_json_value_dumps( L, -1, _buffer, _offset, _capacity ) == HB_FAILURE )
                    {
                        return HB_FAILURE;
                    }

                    lua_pop( L, 1 );

                    it = lua_next( L, -2 );

                    if( it == 0 )
                    {
                        break;
                    }

                    *_offset += sprintf( _buffer + *_offset, ", " );
                }

                lua_pop( L, 1 );

                *_offset += sprintf( _buffer + *_offset, "]" );
            }
        }break;
    default:
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_json_dumps( lua_State * L, int32_t _index, char * _buffer, size_t _capacity, size_t * _size )
{
    size_t offset = 0;

    if( __hb_script_json_value_dumps( L, _index, _buffer, &offset, _capacity ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( _size != HB_NULLPTR )
    {
        *_size = offset;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_json_visitor( const char * _key, hb_json_handle_t * _value, void * _ud )
{
    lua_State * L = (lua_State *)_ud;

    hb_json_type_t json_value_type = hb_json_get_type( _value );

    switch( json_value_type )
    {
    case e_hb_json_object:
        {
            //ToDo

            return HB_FAILURE;
        }break;
    case e_hb_json_array:
        {
            //ToDo

            return HB_FAILURE;
        }break;
    case e_hb_json_string:
        {
            size_t length;
            const char * value;
            if( hb_json_to_string( _value, &value, &length ) == HB_FAILURE )
            {
                return HB_FAILURE;
            }

            lua_pushlstring( L, value, length );
        }break;
    case e_hb_json_integer:
        {
            int64_t value;
            if( hb_json_to_int64( _value, &value ) == HB_FAILURE )
            {
                return HB_FAILURE;
            }

            lua_pushinteger( L, (lua_Integer)value );
        }break;
    case e_hb_json_real:
        {
            double value;
            if( hb_json_to_real( _value, &value ) == HB_FAILURE )
            {
                return HB_FAILURE;
            }

            lua_pushnumber( L, (lua_Number)value );
        }break;
    case e_hb_json_true:
        {
            lua_pushboolean( L, 1 );
        }break;
    case e_hb_json_false:
        {
            lua_pushboolean( L, 0 );
        }break;
    case e_hb_json_null:
        {
            lua_pushnil( L );
        }break;
    default:
        return HB_FAILURE;
        break;
    }

    lua_setfield( L, -2, _key );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_json_loads( lua_State * L, const char * _buffer, size_t _size )
{
    if( _size == 0 )
    {
        lua_createtable( L, 0, 0 );

        return HB_SUCCESSFUL;
    }

    hb_json_handle_t * json_data;
    if( hb_json_create( _buffer, _size, &json_data ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    uint32_t json_count = hb_json_get_fields_count( json_data );

    lua_createtable( L, 0, json_count );

    hb_result_t result = hb_json_foreach( json_data, &__hb_json_visitor, (void *)L );

    hb_json_destroy( json_data );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_json_load_fields( lua_State * L, const char * _buffer, size_t _size, const char ** _fields, uint32_t _fieldcount )
{
    if( _fieldcount == 0 )
    {
        return HB_SUCCESSFUL;
    }

    if( _size == 0 )
    {
        return HB_FAILURE;
    }

    hb_json_handle_t * json_data;
    if( hb_json_create( _buffer, _size, &json_data ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    for( uint32_t index = 0; index != _fieldcount; ++index )
    {
        const char * field = _fields[index];

        hb_json_handle_t * json_field = HB_NULLPTR;
        if( hb_json_get_field( json_data, field, &json_field ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_json_type_t json_field_type = hb_json_get_type( json_field );

        switch( json_field_type )
        {
        case e_hb_json_object:
            {
                //ToDo

                return HB_FAILURE;
            }break;
        case e_hb_json_array:
            {
                //ToDo

                return HB_FAILURE;
            }break;
        case e_hb_json_string:
            {
                size_t length;
                const char * value;
                if( hb_json_to_string( json_field, &value, &length ) == HB_FAILURE )
                {
                    return HB_FAILURE;
                }

                lua_pushlstring( L, value, length );
            }break;
        case e_hb_json_integer:
            {
                int64_t value;
                if( hb_json_to_int64( json_field, &value ) == HB_FAILURE )
                {
                    return HB_FAILURE;
                }

                lua_pushinteger( L, (lua_Integer)value );
            }break;
        case e_hb_json_real:
            {
                double value;
                if( hb_json_to_real( json_field, &value ) == HB_FAILURE )
                {
                    return HB_FAILURE;
                }

                lua_pushnumber( L, (lua_Number)value );
            }break;
        case e_hb_json_true:
            {
                lua_pushboolean( L, 1 );
            }break;
        case e_hb_json_false:
            {
                lua_pushboolean( L, 0 );
            }break;
        case e_hb_json_null:
            {
                lua_pushnil( L );
            }break;
        default:
            break;
        }

        hb_json_destroy( json_field );
    }

    hb_json_destroy( json_data );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_json_create( lua_State * L, int32_t _index, hb_json_handle_t ** _json )
{
    char json_data[HB_DATA_MAX_SIZE];
    size_t json_size;
    if( hb_script_json_dumps( L, _index, json_data, HB_DATA_MAX_SIZE, &json_size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_create( json_data, json_size, _json ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_json_get_public_data( lua_State * L, int32_t _index, const hb_db_collection_handle_t * _collection, hb_uid_t _uid, uint32_t * _count )
{
    const char * fields[HB_SCRIPT_USER_PUBLIC_DATA_FIELD_MAX];
    uint32_t fields_count = 0;

    if( hb_script_get_fields( L, _index, fields, &fields_count ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    const char * db_fields[] = {"public_data"};

    hb_db_values_handle_t * project_values;
    if( hb_db_get_values( _collection, _uid, db_fields, 1, &project_values, HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    const char * public_data_symbol;
    size_t public_data_symbol_length;

    if( hb_db_get_string_value( project_values, 0, &public_data_symbol, &public_data_symbol_length ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_script_json_load_fields( L, public_data_symbol, public_data_symbol_length, fields, fields_count ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( project_values );

    *_count = fields_count;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_json_set_public_data( lua_State * L, int32_t _index, const hb_db_collection_handle_t * _collection, hb_uid_t _uid )
{
    char json_data[HB_DATA_MAX_SIZE];
    size_t json_data_size;
    if( hb_script_json_dumps( L, _index, json_data, HB_DATA_MAX_SIZE, &json_data_size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_values_handle_t * values_update;
    if( hb_db_create_values( &values_update ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_string_value( values_update, "public_data", HB_UNKNOWN_STRING_SIZE, json_data, json_data_size );

    if( hb_db_update_values( _collection, _uid, values_update ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( values_update );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_json_update_public_data( lua_State * L, int32_t _index, const hb_db_collection_handle_t * _collection, hb_uid_t _uid )
{
    const char * db_fields[] = {"public_data"};

    hb_db_values_handle_t * user_values;
    if( hb_db_get_values( _collection, _uid, db_fields, 1, &user_values, HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    const char * public_data_symbol;
    size_t public_data_symbol_length;

    if( hb_db_get_string_value( user_values, 0, &public_data_symbol, &public_data_symbol_length ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_json_handle_t * json_data;
    if( hb_json_create( public_data_symbol, public_data_symbol_length, &json_data ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( user_values );

    hb_json_handle_t * json_update;
    if( hb_script_json_create( L, _index, &json_update ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_update( json_data, json_update ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    char json_new_data[HB_DATA_MAX_SIZE];
    size_t json_new_data_size;
    if( hb_json_dumps( json_data, json_new_data, HB_DATA_MAX_SIZE, &json_new_data_size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_values_handle_t * update_values;

    if( hb_db_create_values( &update_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_string_value( update_values, "public_data", HB_UNKNOWN_STRING_SIZE, json_new_data, json_new_data_size );

    if( hb_db_update_values( _collection, _uid, update_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( update_values );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_get_fields( lua_State * L, int32_t _index, const char ** _fieds, uint32_t * _count )
{
    uint32_t field_iterator = 0;

    lua_pushvalue( L, _index );
    lua_pushnil( L );
    while( lua_next( L, -2 ) != 0 )
    {
        if( field_iterator == HB_SCRIPT_USER_PUBLIC_DATA_FIELD_MAX )
        {
            return HB_FAILURE;
        }

        const char * value = lua_tostring( L, -1 );
        _fieds[field_iterator++] = value;

        lua_pop( L, 1 );
    }
    lua_pop( L, 1 );

    *_count = field_iterator;

    return HB_SUCCESSFUL;
}