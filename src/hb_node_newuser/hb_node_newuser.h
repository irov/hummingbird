#ifndef HB_NODE_NEWUSER_H_
#define HB_NODE_NEWUSER_H_

#include "hb_config/hb_config.h"

typedef struct hb_node_newuser_in_t
{
    char db_uri[128];

    uint8_t puid[12];

    char login[128];
    char password[128];

} hb_node_newuser_in_t;

typedef struct hb_node_newuser_out_t
{
    uint8_t token[12];

    uint32_t exist;
} hb_node_newuser_out_t;

#endif
