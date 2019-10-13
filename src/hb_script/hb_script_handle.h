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

    hb_db_collection_handle_t db_user_collection;
    hb_db_collection_handle_t db_project_collection;

    uint8_t uuid[12];
    uint8_t puid[12];
} hb_script_handle_t;

#endif