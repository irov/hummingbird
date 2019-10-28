#ifndef HB_SCRIPT_JSON_H_
#define HB_SCRIPT_JSON_H_

#include "hb_config/hb_config.h"

#include "lua.h"

hb_result_t hb_script_json_dumps( lua_State * L, char * _buffer, size_t _capacity, size_t * _size );
hb_result_t hb_script_json_loads( lua_State * L, const char * _buffer, size_t _size );
hb_result_t hb_script_json_load_fields( lua_State * L, const char * _buffer, size_t _size, const char ** _fields, uint32_t _fieldcount );


#endif