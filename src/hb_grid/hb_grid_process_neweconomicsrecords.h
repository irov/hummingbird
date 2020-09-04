#ifndef HB_GRID_PROCESS_NEWECONOMICSRECORDS_H_
#define HB_GRID_PROCESS_NEWECONOMICSRECORDS_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_neweconomicsrecords_in_data_t
{
    hb_token_t token;
    hb_uid_t puid;

    hb_source_t records_source;
    hb_size_t records_source_size;
} hb_grid_process_neweconomicsrecords_in_data_t;

typedef struct hb_grid_process_neweconomicsrecords_out_data_t
{
    uint32_t dummy;
} hb_grid_process_neweconomicsrecords_out_data_t;

hb_result_t hb_grid_process_neweconomicsrecords( hb_grid_process_handle_t * _process, const hb_grid_process_neweconomicsrecords_in_data_t * _in, hb_grid_process_neweconomicsrecords_out_data_t * _out );

#endif
