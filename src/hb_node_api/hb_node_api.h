#ifndef HB_NODE_API_H_
#define HB_NODE_API_H_

#include "hb_node/hb_node.h"

static const uint32_t hb_node_api_magic_number = HB_MAGIC_NUMBER( 'U', 'A', 'P', 'I' );
static const uint32_t hb_node_api_version_number = 1;

typedef enum hb_node_api_category_e
{
    e_hb_node_api,
    e_hb_node_event
} hb_node_api_category_e;

typedef struct hb_node_api_in_t
{
    char cache_uri[128];
    uint32_t cache_port;

    char db_uri[128];

    hb_token_t token;

    hb_node_api_category_e category;
    char method[32];
    
    hb_data_t data;
    size_t data_size;

} hb_node_api_in_t;

typedef struct hb_node_api_out_t
{
    char response_data[HB_GRID_REQUEST_DATA_MAX_SIZE];
    size_t response_size;

} hb_node_api_out_t;

#endif
