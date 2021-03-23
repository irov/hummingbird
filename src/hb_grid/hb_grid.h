#ifndef HB_GRID_H_
#define HB_GRID_H_

#include "hb_config/hb_config.h"

#include "hb_thread/hb_thread.h"
#include "hb_matching/hb_matching.h"
#include "hb_messages/hb_messages.h"
#include "hb_events/hb_events.h"
#include "hb_economics/hb_economics.h"
#include "hb_cache/hb_cache.h"
#include "hb_mutex/hb_mutex.h"

#include "evhttp.h"
#include "event2/thread.h"

typedef struct hb_grid_config_t
{
    char name[32];

    char cache_uri[128];
    uint16_t cache_port;
    uint16_t cache_timeout;

    char db_uri[128];
    uint16_t db_port;

    char log_uri[128];
    uint16_t log_port;
} hb_grid_config_t;

typedef struct hb_grid_mutex_handle_t
{
    hb_mutex_handle_t * mutex;
} hb_grid_mutex_handle_t;

typedef struct hb_grid_process_handle_t
{
    char grid_uri[128];
    ev_uint16_t grid_port;

    evutil_socket_t * ev_socket;
    hb_mutex_handle_t * mutex_ev_socket;

    hb_thread_handle_t * thread;
    hb_db_handle_t * db;
    hb_db_client_handle_t * db_client;

    hb_grid_config_t * config;
    
    hb_matching_handle_t * matching;
    hb_messages_handle_t * messages;
    hb_events_handle_t * events;
    hb_economics_handle_t * economics;

    hb_grid_mutex_handle_t * mutex_handles;
    uint32_t mutex_count;

    hb_cache_handle_t * cache;
} hb_grid_process_handle_t;

typedef struct hb_grid_process_cmd_args_t
{    
    char arg1[128];
    char arg2[128];
    char arg3[128];
} hb_grid_process_cmd_args_t;

void hb_grid_process_lock( hb_grid_process_handle_t * _process, hb_uid_t _uuid );
void hb_grid_process_unlock( hb_grid_process_handle_t * _process, hb_uid_t _uuid );

#endif