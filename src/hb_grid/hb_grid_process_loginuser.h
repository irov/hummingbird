#ifndef HB_GRID_PROCESS_LOGINUSER_H_
#define HB_GRID_PROCESS_LOGINUSER_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_loginuser_in_data_t
{
    hb_pid_t puid;

    char login[128];
    char password[128];

} hb_grid_process_loginuser_in_data_t;

typedef struct hb_grid_process_loginuser_out_data_t
{
    hb_token_t token;

    hb_pid_t uuid;
    hb_bool_t exist;
} hb_grid_process_loginuser_out_data_t;

hb_result_t hb_grid_process_loginuser( hb_grid_process_handle_t * _process, const hb_grid_process_loginuser_in_data_t * _data, hb_grid_process_loginuser_out_data_t * _out );

#endif
