#ifndef HB_GRID_PROCESS_NEWACCOUNT_H_
#define HB_GRID_PROCESS_NEWACCOUNT_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_newaccount_in_data_t
{
    char account_login[128 + 1];
    char account_password[128 + 1];

} hb_grid_process_newaccount_in_data_t;

typedef struct hb_grid_process_newaccount_out_data_t
{
    hb_error_code_t code;
} hb_grid_process_newaccount_out_data_t;

hb_result_t hb_grid_process_newaccount( hb_grid_process_handle_t * _process, const hb_grid_process_newaccount_in_data_t * _in, hb_grid_process_newaccount_out_data_t * const _out );

#endif
