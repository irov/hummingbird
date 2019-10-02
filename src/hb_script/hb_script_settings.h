#ifndef HB_SCRIPT_SETTINGS_H_
#define HB_SCRIPT_SETTINGS_H_

#include "hb_db/hb_db.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <setjmp.h>

typedef struct hb_script_settings_t
{
    lua_State * L;

    jmp_buf panic_jump;

    char user[32];

    hb_db_collection_handler_t db_collection;
} hb_script_settings_t;

#endif