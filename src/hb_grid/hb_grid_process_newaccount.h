#ifndef HB_GRID_PROCESS_NEWACCOUNT_H_
#define HB_GRID_PROCESS_NEWACCOUNT_H_

#include "hb_config/hb_config.h"

typedef struct hb_grid_process_newaccount_in_data_t
{
    char login[128];
    char password[128];

} hb_grid_process_newaccount_in_data_t;

typedef struct hb_grid_process_newaccount_out_data_t
{
    hb_token_t token;

    uint32_t exist;
} hb_grid_process_newaccount_out_data_t;

hb_result_t hb_grid_process_newaccount( const hb_grid_process_newaccount_in_data_t * _in, hb_grid_process_newaccount_out_data_t * _out );

#endif
