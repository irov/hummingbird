#ifndef HB_SCRIPT_HANDLE_H_
#define HB_SCRIPT_HANDLE_H_

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

    const hb_db_collection_handle_t * db_user_collection;
    const hb_db_collection_handle_t * db_project_collection;

    hb_oid_t user_oid;
    hb_oid_t project_oid;
} hb_script_handle_t;

#endif