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
static int writer( lua_State * L, const void * p, size_t size, void * u )
{
    HB_UNUSED( L );

    hb_script_writer_desc_t * desc = (hb_script_writer_desc_t *)u;    

    if( desc->carriage + size > desc->capacity )
    {
        return LUA_ERRMEM;
    }

    memcpy( (uint8_t *)desc->buffer + desc->carriage, p, size );

    desc->carriage += size;

    return LUA_OK;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_compiler( const void * _source, size_t _size, void * _code, size_t _capacity, size_t * _compilesize )
{
    lua_State * L = luaL_newstate();

    int ret_loadbuffer = luaL_loadbufferx( L, _source, _size, "compiler", HB_NULLPTR );

    if( ret_loadbuffer != LUA_OK )
    {
        luaL_error( L, "invalid load source" );

        return HB_FAILURE;
    }

    const Proto * f = getproto( L->top - 1 );

    hb_script_writer_desc_t desc;
    desc.carriage = 0;
    desc.capacity = _capacity;
    desc.buffer = _code;

    luaU_dump( L, f, writer, &desc, 1 );

    lua_close( L );
    
    *_compilesize = desc.carriage;
    
    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////