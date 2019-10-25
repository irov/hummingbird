#ifndef HB_NODE_LOGINUSER_H_
#define HB_NODE_LOGINUSER_H_

#include "hb_config/hb_config.h"

static const uint32_t hb_node_loginuser_magic_number = HB_MAGIC_NUMBER( 'N', 'L', 'U', 'R' );
static const uint32_t hb_node_loginuser_version_number = 1;

typedef struct hb_node_loginuser_in_t
{
    uint32_t magic_number;
    uint32_t version_number;

    char cache_uri[128];
    uint32_t cache_port;

    char db_uri[128];

    uint16_t pid;

    char login[128];
    char password[128];

} hb_node_loginuser_in_t;

typedef struct hb_node_loginuser_out_t
{
    uint32_t magic_number;
    uint32_t version_number;

    hb_token_t token;

    uint32_t exist;
} hb_node_loginuser_out_t;

#endif
