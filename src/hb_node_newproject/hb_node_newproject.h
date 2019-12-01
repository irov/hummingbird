#ifndef HB_NODE_NEWPROJECT_H_
#define HB_NODE_NEWPROJECT_H_

#include "hb_node_main/hb_node_main.h"

typedef struct hb_node_newproject_in_t
{
    hb_token_t token;
} hb_node_newproject_in_t;

typedef struct hb_node_newproject_out_t
{
    hb_pid_t pid;
} hb_node_newproject_out_t;

#endif
