#ifndef HB_MATCHING_H_
#define HB_MATCHING_H_

#include "hb_config/hb_config.h"

#include "hb_utils/hb_hashtable.h"

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

typedef struct hb_matching_t
{
    hb_hashtable_t * ht;
} hb_matching_t;

hb_result_t hb_matching_initialize( uint32_t _count, hb_matching_t ** _matching );
void hb_matching_finalize( hb_matching_t * _matching );

hb_result_t hb_matching_create( hb_matching_t * _matching, hb_oid16_t _moid16, hb_oid16_t _uoid16 );

#endif