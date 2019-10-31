#ifndef HB_NODE_UPLOAD_H_
#define HB_NODE_UPLOAD_H_

#include "hb_node/hb_node.h"

static const hb_magic_t hb_node_upload_magic_number = HB_MAGIC_NUMBER( 'U', 'P', 'L', 'D' );
static const hb_version_t hb_node_upload_version_number = 1;

typedef struct hb_node_upload_in_t
{
    char db_uri[128];

    uint16_t pid;

    hb_data_t data;
    size_t data_size;
} hb_node_upload_in_t;

typedef struct hb_node_upload_out_t
{
    int64_t revision;
} hb_node_upload_out_t;

#endif
