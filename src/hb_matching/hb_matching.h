#ifndef HB_MATCHING_H_
#define HB_MATCHING_H_

#include "hb_config/hb_config.h"
#include "hb_thread/hb_thread.h"

#include "evhttp.h"
#include "event2/thread.h"

typedef struct hb_matching_config_t
{
    char name[32];

    char cache_uri[128];
    uint16_t cache_port;
    uint16_t cache_timeout;

    char db_uri[128];
    uint16_t db_port;

    char log_uri[128];
    uint16_t log_port;
} hb_matching_config_t;

typedef struct hb_matching_process_handle_t
{
    char matching_uri[128];
    ev_uint16_t matching_port;

    evutil_socket_t * ev_socket;

    hb_thread_handle_t * thread;

    hb_matching_config_t * config;
} hb_matching_process_handle_t;

#endif