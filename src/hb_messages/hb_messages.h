#ifndef HB_MESSAGES_H_
#define HB_MESSAGES_H_

#include "hb_config/hb_config.h"

#include "hb_cache/hb_cache.h"
#include "hb_db/hb_db.h"

typedef struct hb_messages_handle_t hb_messages_handle_t;

hb_result_t hb_messages_create( hb_messages_handle_t ** _handle );
void hb_messages_destroy( hb_messages_handle_t * _handle );

//hb_result_t hb_messages_new_channel( hb_messages_handle_t * _handle, const hb_uid_t * _uid  );

#endif