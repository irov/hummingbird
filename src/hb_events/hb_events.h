#ifndef HB_EVENTS_H_
#define HB_EVENTS_H_

#include "hb_config/hb_config.h"
#include "hb_cache/hb_cache.h"
#include "hb_db/hb_db.h"

typedef struct hb_events_handle_t hb_events_handle_t;

hb_result_t hb_events_create( hb_events_handle_t ** _handle );
void hb_events_destroy( hb_events_handle_t * _handle );

hb_result_t hb_events_new_topic( hb_events_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, const char * _name, uint32_t _delay, hb_uid_t * _tuid );

typedef struct hb_events_topic_t
{
    uint32_t index;
    const char * message;
}hb_events_topic_t;

hb_result_t hb_events_get_topic( hb_events_handle_t * _handle, const hb_cache_handle_t * _cache, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _tuid, hb_events_topic_t * _topic, hb_error_code_t * _code );

#endif