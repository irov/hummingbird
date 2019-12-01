#ifndef HB_NODE_UPLOAD_H_
#define HB_NODE_UPLOAD_H_

#include "hb_node_main/hb_node_main.h"

typedef struct hb_node_upload_in_t
{
    hb_token_t token;

    hb_pid_t pid;

    hb_source_t script_source;
    size_t script_source_size;
} hb_node_upload_in_t;

typedef struct hb_node_upload_out_t
{
    int64_t revision;
} hb_node_upload_out_t;

#endif
