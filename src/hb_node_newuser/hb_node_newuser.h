#ifndef HB_NODE_NEWUSER_H_
#define HB_NODE_NEWUSER_H_

#include "hb_config/hb_config.h"

static const uint32_t hb_node_newuser_magic_number = HB_MAGIC_NUMBER( 'N', 'N', 'U', 'R' );
static const uint32_t hb_node_newuser_version_number = 1;

typedef struct hb_node_newuser_in_t
{
    uint32_t magic_number;
    uint32_t version_number;

    char db_uri[128];

    uint16_t pid;

    char login[128];
    char password[128];

} hb_node_newuser_in_t;

typedef struct hb_node_newuser_out_t
{
    uint32_t magic_number;
    uint32_t version_number;

    uint8_t token[12];

    uint32_t exist;
} hb_node_newuser_out_t;

#endif
