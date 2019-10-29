#ifndef HB_NODE_NEWUSER_H_
#define HB_NODE_NEWUSER_H_

#include "hb_node/hb_node.h"

static const hb_magic_t hb_node_newuser_magic_number = HB_MAGIC_NUMBER( 'N', 'N', 'U', 'R' );
static const hb_version_t hb_node_newuser_version_number = 1;

typedef struct hb_node_newuser_in_t
{
    char cache_uri[128];
    uint32_t cache_port;

    char db_uri[128];

    uint16_t pid;

    char login[128];
    char password[128];

} hb_node_newuser_in_t;

typedef struct hb_node_newuser_out_t
{
    hb_token_t token;

    uint32_t exist;
} hb_node_newuser_out_t;

#endif
