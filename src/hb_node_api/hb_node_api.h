#ifndef HB_NODE_API_H_
#define HB_NODE_API_H_

#include "hb_config/hb_config.h"

typedef struct hb_node_api_in_t
{
    char db_uri[128];

    uint8_t token[12];

    char method[128];
    
    char data[10240];
    size_t data_size;

} hb_node_api_in_t;

typedef struct hb_node_api_out_t
{
    char result[10240];
    size_t result_size;

} hb_node_api_out_t;

#endif
