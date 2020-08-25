#ifndef HB_GRID_PROCESS_LOGINUSER_H_
#define HB_GRID_PROCESS_LOGINUSER_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_loginuser_in_data_t
{
    hb_uid_t puid;

    char login[128];
    char password[128];

} hb_grid_process_loginuser_in_data_t;

typedef struct hb_grid_process_loginuser_out_data_t
{
    hb_token_t token;

    int32_t project_public_data_revision;

    hb_uid_t uuid;
    hb_error_code_t code;

    char user_public_data[HB_DATA_MAX_SIZE];
} hb_grid_process_loginuser_out_data_t;

hb_result_t hb_grid_process_loginuser( hb_grid_process_handle_t * _process, const hb_grid_process_loginuser_in_data_t * _data, hb_grid_process_loginuser_out_data_t * _out );

#endif
