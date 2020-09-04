#ifndef HB_SCRIPT_HANDLE_H_
#define HB_SCRIPT_HANDLE_H_

#include "hb_config/hb_config.h"

#include "hb_db/hb_db.h"
#include "hb_cache/hb_cache.h"
#include "hb_matching/hb_matching.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <setjmp.h>

typedef struct hb_script_handle_t
{
    lua_State * L;

    jmp_buf panic_jump;

    hb_size_t memory_base;
    hb_size_t memory_used;
    hb_size_t memory_peak;
    hb_size_t memory_limit;

    uint32_t call_used;
    uint32_t call_limit;

    const hb_cache_handle_t * cache;

    const hb_db_client_handle_t * db_client;

    hb_db_collection_handle_t * db_collection_projects;
    hb_db_collection_handle_t * db_collection_users;
    hb_db_collection_handle_t * db_collection_project_entities;
    hb_db_collection_handle_t * db_collection_user_entities;
    hb_db_collection_handle_t * db_collection_matching;
    hb_db_collection_handle_t * db_collection_worlds;
    hb_db_collection_handle_t * db_collection_avatars;
    hb_db_collection_handle_t * db_collection_scripts;

    hb_uid_t project_uid;
    hb_uid_t user_uid;

    hb_matching_handle_t * matching;
} hb_script_handle_t;

void hb_script_error( lua_State * L, const char * _format, ... );

#define HB_SCRIPT_ERROR(L, ...) hb_script_error(L, __VA_ARGS__ )

#ifdef HB_DEBUG
#define HB_SCRIPT_ERROR_INTERNAL_ERROR(L) hb_script_error(L, "internal error in '%s' [%u]", HB_CODE_FILE, HB_CODE_LINE )
#else
#define HB_SCRIPT_ERROR_INTERNAL_ERROR(L) hb_script_error(L, "internal error" )
#endif

#endif