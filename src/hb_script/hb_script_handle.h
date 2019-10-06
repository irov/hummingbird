#ifndef HB_SCRIPT_SETTINGS_H_
#define HB_SCRIPT_SETTINGS_H_

#include "hb_db/hb_db.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <setjmp.h>

typedef struct hb_script_handle_t
{
    lua_State * L;

    jmp_buf panic_jump;

    size_t memory_base;
    size_t memory_used;
    size_t memory_peak;
    size_t memory_limit;

    uint32_t call_used;
    uint32_t call_limit;

    char user[32];

    hb_db_collection_handle_t db_collection;
} hb_script_handle_t;

#endif