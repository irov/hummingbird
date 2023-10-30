#ifndef HB_GRID_CMD_H_
#define HB_GRID_CMD_H_

#include "hb_config/hb_config.h"

#include "hb_json/hb_json.h"

//////////////////////////////////////////////////////////////////////////
typedef struct hb_grid_cmd_inittab_t
{
    const char * name;
    hb_http_code_t( *request )(hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size);
} hb_grid_cmd_inittab_t;
//////////////////////////////////////////////////////////////////////////
extern hb_http_code_t hb_grid_request_newaccount( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_loginaccount( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_newproject( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_upload( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_newuser( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_loginuser( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_api( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_avatar( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_command( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_setusernickname( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_setleaderscore( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_getleaderrank( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_getleaderboard( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_newmessageschannel( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_postmessageschannel( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_getmessageschannel( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_neweventstopic( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
extern hb_http_code_t hb_grid_request_geteventstopic( hb_grid_process_handle_t * _process, hb_json_handle_t * _data, char * _response, hb_size_t * _size );
//////////////////////////////////////////////////////////////////////////
static hb_grid_cmd_inittab_t grid_cmds[] =
{
    { "newaccount", &hb_grid_request_newaccount },
    { "loginaccount", &hb_grid_request_loginaccount },
    { "newproject", &hb_grid_request_newproject },
    { "upload", &hb_grid_request_upload },
    { "newuser", &hb_grid_request_newuser },
    { "loginuser", &hb_grid_request_loginuser },
    { "api", &hb_grid_request_api },
    { "setusernickname", &hb_grid_request_setusernickname },
    { "setleaderscore", &hb_grid_request_setleaderscore },
    { "getleaderrank", &hb_grid_request_getleaderrank },
    { "getleaderboard", &hb_grid_request_getleaderboard },
    { "newmessageschannel", &hb_grid_request_newmessageschannel },
    { "postmessageschannel", &hb_grid_request_postmessageschannel },
    { "getmessageschannel", &hb_grid_request_getmessageschannel },
    { "avatar", &hb_grid_request_avatar },
    { "command", &hb_grid_request_command },
    { "neweventstopic", &hb_grid_request_neweventstopic },
    { "geteventstopic", &hb_grid_request_geteventstopic },
};
//////////////////////////////////////////////////////////////////////////

#endif