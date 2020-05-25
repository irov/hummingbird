#ifndef HB_GRID_CMD_H_
#define HB_GRID_CMD_H_

#include "hb_grid/hb_grid.h"
#include "hb_config/hb_config.h"

//////////////////////////////////////////////////////////////////////////
typedef struct hb_grid_cmd_inittab_t
{
    char * name;
    hb_http_code_t(*request)(struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _cmd);
    int32_t args;
} hb_grid_cmd_inittab_t;
//////////////////////////////////////////////////////////////////////////
extern hb_http_code_t hb_grid_request_newaccount( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _cmd );
extern hb_http_code_t hb_grid_request_loginaccount( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _cmd );
extern hb_http_code_t hb_grid_request_newproject( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _cmd );
extern hb_http_code_t hb_grid_request_upload( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _cmd );
extern hb_http_code_t hb_grid_request_newuser( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _cmd );
extern hb_http_code_t hb_grid_request_loginuser( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _cmd );
extern hb_http_code_t hb_grid_request_api( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _cmd );
extern hb_http_code_t hb_grid_request_avatar( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _cmd );
extern hb_http_code_t hb_grid_request_command( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _cmd );
extern hb_http_code_t hb_grid_request_setusernickname( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _cmd );
extern hb_http_code_t hb_grid_request_setleaderscore( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _cmd );
extern hb_http_code_t hb_grid_request_getleaderrank( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _cmd );
extern hb_http_code_t hb_grid_request_getleaderboard( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _cmd );
extern hb_http_code_t hb_grid_request_newmessageschannel( struct evhttp_request * _request, hb_grid_process_handle_t * _process, char * _response, size_t * _size, const hb_grid_process_cmd_args_t * _args );
//////////////////////////////////////////////////////////////////////////
static hb_grid_cmd_inittab_t grid_cmds[] =
{
    { "newaccount", &hb_grid_request_newaccount, 0 },
    { "loginaccount", &hb_grid_request_loginaccount, 0 },
    { "newproject", &hb_grid_request_newproject, 1 },
    { "upload", &hb_grid_request_upload, 2 },
    { "newuser", &hb_grid_request_newuser, 1 },
    { "loginuser", &hb_grid_request_loginuser, 1 },
    { "api", &hb_grid_request_api, 2 },
    { "setusernickname", &hb_grid_request_setusernickname, 1 },
    { "setleaderscore", &hb_grid_request_setleaderscore, 1 },
    { "getleaderrank", &hb_grid_request_getleaderrank, 1 },
    { "getleaderboard", &hb_grid_request_getleaderboard, 1 },
    { "newmessageschannel", &hb_grid_request_newmessageschannel, 1},
    { "avatar", &hb_grid_request_avatar, 2 },
    { "command", &hb_grid_request_command, 3 }
};
//////////////////////////////////////////////////////////////////////////

#endif