#ifndef HB_MESSAGES_H_
#define HB_MESSAGES_H_

#include "hb_config/hb_config.h"

#include "hb_cache/hb_cache.h"
#include "hb_db/hb_db.h"

typedef struct hb_messages_handle_t hb_messages_handle_t;

hb_result_t hb_messages_create( const hb_db_client_handle_t * _client, hb_messages_handle_t ** _handle );
void hb_messages_destroy( hb_messages_handle_t * _handle );

hb_result_t hb_messages_new_channel( hb_messages_handle_t * _handle, hb_uid_t _puid, hb_uid_t * _muid );

typedef struct hb_messages_post_t
{
    char message[256];
    size_t len;

    uint32_t replay_id;
    hb_uid_t target_uuid;
} hb_messages_post_t;

hb_result_t hb_messages_channel_new_message( hb_messages_handle_t * _handle, hb_uid_t _puid, hb_uid_t _muid, hb_uid_t _uuid, const hb_messages_post_t * _post, hb_error_code_t * _code );

#endif