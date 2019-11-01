#ifndef HB_NODE_LOGINUSER_H_
#define HB_NODE_LOGINUSER_H_

#include "hb_node/hb_node.h"

static const hb_magic_t hb_node_loginuser_magic_number = HB_MAGIC_NUMBER( 'N', 'L', 'U', 'R' );
static const hb_version_t hb_node_loginuser_version_number = 1;

typedef struct hb_node_loginuser_in_t
{
    char cache_uri[128];
    uint16_t cache_port;

    char db_uri[128];

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
