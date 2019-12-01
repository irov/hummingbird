#ifndef HB_GRID_H_
#define HB_GRID_H_

#include "hb_config/hb_config.h"
#include "hb_node/hb_node.h"
#include "hb_thread/hb_thread.h"
#include "hb_sharedmemory/hb_sharedmemory.h"

#include "evhttp.h"
#include "event2/thread.h"

typedef struct hb_grid_process_handle_t
{
    char grid_uri[128];
    ev_uint16_t grid_port;

    evutil_socket_t * ev_socket;

    hb_thread_handle_t * thread;
    hb_sharedmemory_handle_t * sharedmemory;

    hb_node_config_t * config;
} hb_grid_process_handle_t;

#endif