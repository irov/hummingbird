#ifndef HB_NODE_API_H_
#define HB_NODE_API_H_

#include "hb_config/hb_config.h"

static const uint32_t hb_node_api_magic_number = HB_MAGIC_NUMBER( 'U', 'A', 'P', 'I' );
static const uint32_t hb_node_api_version_number = 1;

typedef struct hb_node_api_in_t
{
    uint32_t magic_number;
    uint32_t version_number;

    char cache_uri[128];
    uint32_t cache_port;

    char db_uri[128];

    hb_token_t token;

    char method[32];
    
    hb_data_t data;
    size_t data_size;

} hb_node_api_in_t;

typedef struct hb_node_api_out_t
{
    uint32_t magic_number;
    uint32_t version_number;

    char data[HB_GRID_REQUEST_DATA_MAX_SIZE];

} hb_node_api_out_t;

#endif
