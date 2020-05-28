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

    size_t memory_base;
    size_t memory_used;
    size_t memory_peak;
    size_t memory_limit;

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

    hb_uid_t project_oid;
    hb_uid_t user_oid;

    hb_matching_handle_t * matching;
} hb_script_handle_t;

void hb_script_error( lua_State * L, const char * _format, ... );

#define HB_SCRIPT_ERROR(L, ...) hb_script_error(L, __VA_ARGS__ )

#endif