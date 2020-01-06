#include "hb_script_json.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_script_json_value_dumps( lua_State * L, int _index, char * _buffer, size_t * _offset, size_t _capacity )
{
    HB_UNUSED( _capacity );

    int type = lua_type( L, _index );

    switch( type )
    {
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
            size_t len = lua_rawlen( L, _index );

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

    *_size = offset;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static void __hb_json_visitor( const char * _key, hb_json_handle_t * _value, void * _ud )
{
    lua_State * L = (lua_State *)_ud;

    hb_json_type_t json_value_type = hb_json_get_type( _value );

    switch( json_value_type )
    {
    case e_hb_json_object:
        {
            //ToDo
        }break;
    case e_hb_json_array:
        {
            //ToDo
        }break;
    case e_hb_json_string:
        {
            size_t length;
            const char * value;
            hb_json_to_string( _value, &value, &length );

            lua_pushlstring( L, value, length );
        }break;
    case e_hb_json_integer:
        {
            int64_t value;
            hb_json_to_integer( _value, &value );

            lua_pushinteger( L, (lua_Integer)value );
        }break;
    case e_hb_json_real:
        {
            double value;
            hb_json_to_real( _value, &value );

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

    lua_setfield( L, -2, _key );
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

    hb_json_foreach( json_data, &__hb_json_visitor, (void *)L );

    hb_json_destroy( json_data );

    return HB_SUCCESSFUL;
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
            }break;
        case e_hb_json_array:
            {
                //ToDo
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
                if( hb_json_to_integer( json_field, &value ) == HB_FAILURE )
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
    char json_data[10240];
    size_t json_size;
    if( hb_script_json_dumps( L, _index, json_data, 10240, &json_size ) == HB_FAILURE )
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