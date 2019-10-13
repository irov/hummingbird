#ifndef HB_NODE_UNIVERSE_H_
#define HB_NODE_UNIVERSE_H_

#include "hb_config/hb_config.h"

typedef struct hb_node_universe_in_t
{
    char db_uri[128];

    uint32_t token_expire_time;

} hb_node_universe_in_t;

typedef struct hb_node_universe_out_t
{
    uint32_t dummy;
} hb_node_universe_out_t;

#endif
