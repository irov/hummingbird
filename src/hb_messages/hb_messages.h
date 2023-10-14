#ifndef HB_MESSAGES_H_
#define HB_MESSAGES_H_

#include "hb_config/hb_config.h"
#include "hb_db/hb_db.h"

typedef struct hb_messages_handle_t hb_messages_handle_t;

hb_result_t hb_messages_create( hb_messages_handle_t ** _handle );
void hb_messages_destroy( hb_messages_handle_t * _handle );

hb_result_t hb_messages_new_channel( hb_messages_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, uint32_t _maxpost, hb_uid_t * _cuid );

typedef struct hb_messages_post_t
{
    hb_uid_t user_uid;
    const char * message;
    const char * metainfo;
} hb_messages_post_t;

hb_result_t hb_messages_channel_new_post( hb_messages_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _cuid, const hb_messages_post_t * _post, uint32_t * _postid, hb_error_code_t * _code );

typedef struct hb_messages_get_t
{
    uint32_t postid;
    hb_uid_t user_uid;
    const char * message;
    const char * metainfo;
} hb_messages_get_t;

hb_result_t hb_messages_channel_get_posts( hb_messages_handle_t * _handle, hb_uid_t _puid, hb_uid_t _cuid, uint32_t _postid, hb_messages_get_t * _post, hb_size_t _postcapacity, uint32_t * _count, hb_error_code_t * _code );

#endif