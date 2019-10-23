#ifndef HB_NODE_NEWPROJECT_H_
#define HB_NODE_NEWPROJECT_H_

#include "hb_config/hb_config.h"

static const uint32_t hb_node_newproject_magic_number = HB_MAGIC_NUMBER( 'N', 'N', 'P', 'J' );
static const uint32_t hb_node_newproject_version_number = 1;

typedef struct hb_node_newproject_in_t
{
    uint32_t magic_number;
    uint32_t version_number;
    char db_uri[128];
} hb_node_newproject_in_t;

typedef struct hb_node_newproject_out_t
{
    uint32_t magic_number;
    uint32_t version_number;
    int32_t pid;
} hb_node_newproject_out_t;

#endif
