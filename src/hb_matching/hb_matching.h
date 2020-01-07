#ifndef HB_MATCHING_H_
#define HB_MATCHING_H_

#include "hb_config/hb_config.h"

#include "hb_utils/hb_hashtable.h"
#include "hb_utils/hb_array.h"

typedef struct hb_matching_user_t
{
    hb_oid_t uoid;
    int32_t rating;
    hb_pid_t apid;

    hb_array_t * public_data;
} hb_matching_user_t;

typedef struct hb_matching_room_t
{
    uint32_t matching_count;
    uint32_t matching_dispersion;

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

typedef struct hb_matching_complete_desc_t
{
    const char * name;

    hb_pid_t wpid;

    const hb_matching_user_t * users;
    uint32_t users_count;
    
    const char * data;
    size_t data_size;

    void * ud;
} hb_matching_complete_desc_t;

typedef hb_result_t( *hb_matching_complete_func_t )(const hb_matching_complete_desc_t * _desc);
hb_result_t hb_matching_join( hb_matching_t * _matching, hb_oid_t _poid, const char * _name, size_t _namesize, hb_oid_t _uoid, int32_t _rating, const void * _data, size_t _datasize, hb_bool_t * _exist, hb_matching_complete_func_t _complete, void * _ud );

#endif