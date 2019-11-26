#ifndef HB_NODE_API_H_
#define HB_NODE_API_H_

#include "hb_node_main/hb_node_main.h"

typedef enum hb_node_api_category_e
{
    e_hb_node_api,
    e_hb_node_event
} hb_node_api_category_e;

typedef struct hb_node_api_in_t
{
    hb_token_t token;

    hb_node_api_category_e category;
    char method[32];
    
    hb_data_t data;
    size_t data_size;

} hb_node_api_in_t;

typedef struct hb_node_api_out_t
{
    hb_bool_t successful;
    hb_bool_t method_found;

    char response_data[HB_GRID_REQUEST_DATA_MAX_SIZE];
    size_t response_size;

    size_t memory_used;
    uint32_t call_used;

} hb_node_api_out_t;

#endif
