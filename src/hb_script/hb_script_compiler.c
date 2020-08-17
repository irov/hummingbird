#include "hb_script_compiler.h"

#include "hb_log/hb_log.h"

#include "hb_script_handle.h"

#include "lua.h"

#include "lstate.h"
#include "lundump.h"

#include <memory.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_script_writer_desc_t
{
    size_t carriage;
    size_t capacity;

    void * buffer;
}hb_script_writer_desc_t;
//////////////////////////////////////////////////////////////////////////
static int s_writer( lua_State * L, const void * p, size_t size, void * u )
{
    HB_UNUSED( L );

    hb_script_writer_desc_t * desc = (hb_script_writer_desc_t *)u;

    if( desc->carriage + size > desc->capacity )
    {
        return LUA_ERRMEM;
    }

    memcpy( (hb_byte_t *)desc->buffer + desc->carriage, p, size );

    desc->carriage += size;

    return LUA_OK;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_compiler( const char * _source, size_t _size, void * _code, size_t _capacity, size_t * _compilesize )
{
    lua_State * L = luaL_newstate();

    int ret_loadbuffer = luaL_loadbufferx( L, _source, _size, "compiler", HB_NULLPTR );

    if( ret_loadbuffer != LUA_OK )
    {
        const char * error_msg = lua_tostring( L, -1 );

        HB_LOG_MESSAGE_ERROR( "script", "invalid compile script: %s [%d]"
            , error_msg
            , ret_loadbuffer
        );

        lua_pop( L, 1 );

        lua_close( L );

        return HB_FAILURE;
    }

    TValue * o = s2v( L->top - 1 );
    const Proto * f = getproto( o );

    hb_script_writer_desc_t desc;
    desc.carriage = 0;
    desc.capacity = _capacity;
    desc.buffer = _code;

    int ret_dump = luaU_dump( L, f, &s_writer, &desc, 1 );

    if( ret_dump != LUA_OK )
    {
        const char * error_msg = lua_tostring( L, -1 );

        HB_LOG_MESSAGE_ERROR( "script", "invalid dump code: %s [%d]"
            , error_msg
            , ret_loadbuffer
        );

        lua_pop( L, 1 );

        lua_close( L );

        return HB_FAILURE;
    }

    lua_close( L );

    *_compilesize = desc.carriage;

    HB_LOG_MESSAGE_INFO( "script", "successful compile script" );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////