#ifndef HB_NODE_NEWPROJECT_H_
#define HB_NODE_NEWPROJECT_H_

#include "hb_config/hb_config.h"

typedef struct hb_node_newproject_in_t
{
    char db_uri[128];
} hb_node_newproject_in_t;

typedef struct hb_node_newproject_out_t
{
    uint8_t puid[12];
} hb_node_newproject_out_t;

#endif
