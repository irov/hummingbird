#ifndef HB_GRID_H_
#define HB_GRID_H_

#include "hb_config/hb_config.h"
#include "hb_node/hb_node.h"
#include "hb_thread/hb_thread.h"
#include "hb_sharedmemory/hb_sharedmemory.h"

#pragma warning( push )
#pragma warning( disable : 4706 )
#include "evhttp.h"
#include "event2/thread.h"
#pragma warning( pop )

struct hb_grid_process_handle_t;

typedef int (*hb_grid_request_t)(struct evhttp_request * _request, struct hb_grid_process_handle_t * _handle, char * _response, size_t * _size);

typedef struct hb_grid_request_handle_t
{
    hb_grid_request_t request;
    struct hb_grid_process_handle_t * process;
} hb_grid_request_handle_t;

typedef struct hb_grid_process_handle_t
{
    char grid_uri[128];
    ev_uint16_t grid_port;

    evutil_socket_t * ev_socket;

    hb_thread_handle_t * thread;
    hb_sharedmemory_handle_t * sharedmemory;

    hb_node_config_t * config;

    hb_grid_request_handle_t requests[8];
} hb_grid_process_handle_t;

#endif