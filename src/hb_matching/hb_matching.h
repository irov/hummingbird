#ifndef HB_MATCHING_H_
#define HB_MATCHING_H_

#include "hb_config/hb_config.h"

#include "hb_utils/hb_hashtable.h"

typedef struct hb_matching_user_t
{
    hb_oid_t uoid;
    int32_t rating;
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

hb_result_t hb_matching_create( hb_matching_t * _matching, hb_oid_t _poid, const char * _name, size_t _namesize, uint32_t _count, uint32_t _dispersion, const void * _data, size_t _datasize, hb_bool_t * _exist );

typedef void(*hb_matching_complete_t)(const hb_matching_user_t * _user, int32_t _count, const char * _data, size_t _datasize);
hb_result_t hb_matching_join( hb_matching_t * _matching, hb_oid_t _poid, const char * _name, size_t _namesize, hb_oid_t _uoid, int32_t _rating, hb_bool_t * _exist, hb_matching_complete_t _complete );

#endif