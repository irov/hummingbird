#ifndef HB_MATCHING_H_
#define HB_MATCHING_H_

#include "hb_config/hb_config.h"
#include "hb_thread/hb_thread.h"
#include "hb_db/hb_db.h"
#include "hb_utils/hb_hashtable.h"

#include "evhttp.h"
#include "event2/thread.h"

typedef struct hb_matching_user_t
{
    hb_oid16_t uoid;
    int64_t rating;
} hb_matching_user_t;

typedef struct hb_matching_room_t
{
    int32_t count;
    int32_t dispersion;

    hb_matching_user_t * users;
    size_t users_count;
    size_t users_capacity;
} hb_matching_room_t;

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

    hb_db_collection_handle_t * db_collection_matching;

    hb_hashtable_t * ht;
} hb_matching_process_handle_t;

#endif