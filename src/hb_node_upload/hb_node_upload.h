#ifndef HB_NODE_UPLOAD_H_
#define HB_NODE_UPLOAD_H_

#include "hb_config/hb_config.h"

typedef struct hb_node_upload_in_t
{
    char db_uri[128];

    uint8_t puid[12];

    hb_data_t data;
    size_t data_size;
    
} hb_node_upload_in_t;

typedef struct hb_node_upload_out_t
{
    uint8_t dummy;
} hb_node_upload_out_t;

#endif
