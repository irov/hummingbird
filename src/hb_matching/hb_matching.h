#ifndef HB_MATCHING_H_
#define HB_MATCHING_H_

#include "hb_db/hb_db.h"
#include "hb_json/hb_json.h"

#include "hb_utils/hb_hashtable.h"
#include "hb_utils/hb_array.h"

#include "hb_config/hb_config.h"

typedef struct hb_matching_desc_t
{
    uint32_t matching_count;
    uint32_t matching_dispersion;
    uint32_t join_timeout;
    uint32_t wait_timeout;
} hb_matching_desc_t;

typedef struct hb_matching_user_handle_t hb_matching_user_handle_t;
typedef struct hb_matching_handle_t hb_matching_handle_t;

hb_result_t hb_matching_create( uint32_t _count, hb_matching_handle_t ** _matching );
void hb_matching_destroy( hb_matching_handle_t * _matching );

hb_result_t hb_matching_room_create( hb_matching_handle_t * _matching, const hb_db_client_handle_t * _client, hb_uid_t _puid, const char * _name, size_t _namesize, const hb_matching_desc_t * _desc, const void * _data, size_t _datasize, hb_bool_t * _exist );

typedef struct hb_matching_complete_desc_t
{
    const char * name;

    hb_uid_t wuid;

    const hb_matching_user_handle_t * const * users;
    uint32_t users_count;
    
    const hb_json_handle_t * json_data;

    void * ud;
} hb_matching_complete_desc_t;

typedef hb_result_t( *hb_matching_complete_func_t )(const hb_matching_complete_desc_t * _desc);
hb_result_t hb_matching_join( hb_matching_handle_t * _matching, const hb_db_client_handle_t * _client, hb_uid_t _puid, const char * _name, size_t _namesize, hb_uid_t _uuid, uint32_t _rating, const void * _data, size_t _datasize, hb_bool_t * _exist, hb_matching_complete_func_t _complete, void * _ud );
hb_result_t hb_matching_found( hb_matching_handle_t * _matching, const hb_db_client_handle_t * _client, hb_uid_t _puid, const char * _name, size_t _namesize, hb_uid_t _uuid, hb_bool_t * _exist, hb_uid_t * _auid );
hb_result_t hb_matching_ready( hb_matching_handle_t * _matching, const hb_db_client_handle_t * _client, hb_uid_t _puid, const char * _name, size_t _namesize, hb_uid_t _uuid, hb_uid_t _auid, hb_bool_t * _exist );

hb_uid_t hb_matching_user_get_auid( const hb_matching_user_handle_t * _user );
int32_t hb_matching_user_get_rating( const hb_matching_user_handle_t * _user );
const void * hb_matching_user_get_public_data( const hb_matching_user_handle_t * _user, size_t * _size );

#endif