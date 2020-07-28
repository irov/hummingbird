#ifndef HB_SCRIPT_JSON_H_
#define HB_SCRIPT_JSON_H_

#include "hb_config/hb_config.h"

#include "hb_json/hb_json.h"
#include "hb_db/hb_db.h"

#include "lua.h"

hb_result_t hb_script_json_dumps( lua_State * L, int32_t _index, char * _buffer, size_t _capacity, size_t * _size );
hb_result_t hb_script_json_loads( lua_State * L, const hb_json_handle_t * _json );
hb_result_t hb_script_json_loads_data( lua_State * L, const void * _data, size_t _size );
hb_result_t hb_script_json_load_fields( lua_State * L, const hb_json_handle_t * _json, const char ** _fields, uint32_t _fieldcount );
hb_result_t hb_script_json_load_fields_data( lua_State * L, const void * _data, size_t _size, const char ** _fields, uint32_t _fieldcount );
hb_result_t hb_script_json_create( lua_State * L, int32_t _index, hb_json_handle_t ** _json );

hb_result_t hb_script_json_get_public_data( lua_State * L, int32_t _index, const hb_db_collection_handle_t * _collection, hb_uid_t _uid, uint32_t * _count );
hb_result_t hb_script_json_set_public_data( lua_State * L, int32_t _index, const hb_db_collection_handle_t * _collection, hb_uid_t _uid );
hb_result_t hb_script_json_update_public_data( lua_State * L, int32_t _index, const hb_db_collection_handle_t * _collection, hb_uid_t _uid );

hb_result_t hb_script_get_fields( lua_State * L, int32_t _index, const char ** _fieds, uint32_t * _count );

#endif