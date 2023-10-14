#ifndef HB_GRID_PROCESS_LOGINUSER_H_
#define HB_GRID_PROCESS_LOGINUSER_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_loginuser_in_data_t
{
    hb_uid_t project_uid;

    char login[128];
    char password[128];

    int32_t user_public_data_revision;
    int32_t project_public_data_revision;

} hb_grid_process_loginuser_in_data_t;

typedef struct hb_grid_process_loginuser_out_data_t
{
    hb_token_t token;

    hb_uid_t user_uid;
    hb_error_code_t code;

    char user_public_data[HB_DATA_MAX_SIZE];
    int32_t user_public_data_revision;

    char project_public_data[HB_DATA_MAX_SIZE];
    int32_t project_public_data_revision;
} hb_grid_process_loginuser_out_data_t;

hb_result_t hb_grid_process_loginuser( hb_grid_process_handle_t * _process, const hb_grid_process_loginuser_in_data_t * _data, hb_grid_process_loginuser_out_data_t * const _out );

#endif
