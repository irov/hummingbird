#ifndef HB_GRID_PROCESS_NEWUSER_H_
#define HB_GRID_PROCESS_NEWUSER_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_newuser_in_data_t
{
    hb_pid_t pid;

    char login[128];
    char password[128];

} hb_grid_process_newuser_in_data_t;

typedef struct hb_grid_process_newuser_out_data_t
{
    hb_token_t token;

    uint32_t exist;
} hb_grid_process_newuser_out_data_t;

hb_result_t hb_grid_process_newuser( hb_grid_process_handle_t * _process, const hb_grid_process_newuser_in_data_t * _in, hb_grid_process_newuser_out_data_t * _out );

#endif