#ifndef HB_NODE_LOGINUSER_H_
#define HB_NODE_LOGINUSER_H_

#include "hb_node_main/hb_node_main.h"

typedef struct hb_node_loginuser_in_t
{
    hb_pid_t pid;

    char login[128];
    char password[128];

} hb_node_loginuser_in_t;

typedef struct hb_node_loginuser_out_t
{
    hb_token_t token;

    hb_bool_t exist;
} hb_node_loginuser_out_t;

#endif
