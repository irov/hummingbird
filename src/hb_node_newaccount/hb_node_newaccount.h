#ifndef HB_NODE_NEWACCOUNT_H_
#define HB_NODE_NEWACCOUNT_H_

#include "hb_node_main/hb_node_main.h"

typedef struct hb_node_newaccount_in_t
{
    char login[128];
    char password[128];

} hb_node_newaccount_in_t;

typedef struct hb_node_newaccount_out_t
{
    hb_token_t token;

    uint32_t exist;
} hb_node_newaccount_out_t;

#endif
