#include "hb_matching.h"

#include "hb_memory/hb_memory.h"
#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"

#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_oid.h"

#include <string.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
#ifndef HB_MATCHING_ROOM_USERS_MAX
#define HB_MATCHING_ROOM_USERS_MAX 64
#endif
//////////////////////////////////////////////////////////////////////////
typedef enum hb_matching_user_status_e
{
    e_hb_matching_user_status_join,
    e_hb_matching_user_status_wait,
    e_hb_matching_user_status_ready,
    e_hb_matching_user_status_play
} hb_matching_user_status_e;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_matching_user_handle_t
{
    hb_oid_t uoid;
    int32_t rating;
    hb_pid_t apid;

    hb_array_t * public_data;

    hb_matching_user_status_e status;

    uint32_t timeout;
} hb_matching_user_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_matching_room_handle_t
{
    hb_matching_desc_t desc;

    hb_matching_user_handle_t * users;
    size_t users_count;
    size_t users_capacity;
} hb_matching_room_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_matching_handle_t
{
    hb_hashtable_t * ht;
} hb_matching_handle_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_matching_create( uint32_t _count, hb_matching_handle_t ** _matching )
{
    hb_matching_handle_t * matching = HB_NEW( hb_matching_handle_t );

    hb_hashtable_t * ht;
    hb_hashtable_create( _count, &ht );

    matching->ht = ht;

    *_matching = matching;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_matching_destroy( hb_matching_handle_t * _matching )
{
    hb_hashtable_destroy( _matching->ht );
    HB_DELETE( _matching );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_matching_room_create( hb_matching_handle_t * _matching, const hb_db_client_handle_t * _client, const hb_oid_t * _poid, const char * _name, size_t _namesize, const hb_matching_desc_t * _desc, const void * _data, size_t _datasize, hb_bool_t * _exist )
{
    hb_db_collection_handle_t * db_collection_matching;
    if( hb_db_get_collection( _client, "hb", "hb_matching", &db_collection_matching ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "matching", "invalid initialize script: db not found collection '%s'"
            , "hb_matching"
        );

        return HB_FAILURE;
    }

    if( _namesize == HB_UNKNOWN_STRING_SIZE )
    {
        _namesize = strlen( _name );
    }

    hb_db_values_handle_t * find_values;
    if( hb_db_create_values( &find_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_oid_value( find_values, "poid", HB_UNKNOWN_STRING_SIZE, _poid );
    hb_db_make_symbol_value( find_values, "name", HB_UNKNOWN_STRING_SIZE, _name, _namesize );

    hb_bool_t exist;
    if( hb_db_find_oid( db_collection_matching, find_values, HB_NULLPTR, &exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( find_values );

    if( exist == HB_TRUE )
    {
        *_exist = HB_TRUE;

        return HB_SUCCESSFUL;
    }

    *_exist = HB_FALSE;

    hb_db_values_handle_t * room_values;
    if( hb_db_create_values( &room_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_oid_value( room_values, "poid", HB_UNKNOWN_STRING_SIZE, _poid );
    hb_db_make_symbol_value( room_values, "name", HB_UNKNOWN_STRING_SIZE, _name, _namesize );
    hb_db_make_int32_value( room_values, "count", HB_UNKNOWN_STRING_SIZE, (int32_t)_desc->matching_count );
    hb_db_make_int32_value( room_values, "dispersion", HB_UNKNOWN_STRING_SIZE, (int32_t)_desc->matching_dispersion );
    hb_db_make_int32_value( room_values, "join_timeout", HB_UNKNOWN_STRING_SIZE, (int32_t)_desc->join_timeout );
    hb_db_make_int32_value( room_values, "wait_timeout", HB_UNKNOWN_STRING_SIZE, (int32_t)_desc->wait_timeout );
    hb_db_make_symbol_value( room_values, "public_data", HB_UNKNOWN_STRING_SIZE, _data, _datasize );

    hb_oid_t moid;
    if( hb_db_new_document( db_collection_matching, room_values, &moid ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( room_values );

    hb_matching_room_handle_t * new_room = HB_NEW( hb_matching_room_handle_t );

    new_room->desc = *_desc;

    new_room->users = HB_NEWN( hb_matching_user_handle_t, 64 );
    new_room->users_count = 0;
    new_room->users_capacity = 64;

    hb_hashtable_emplace( _matching->ht, moid.value, sizeof( hb_oid_t ), new_room );

    hb_db_destroy_collection( db_collection_matching );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static int32_t __matching_user_cmp( const void * _left, const void * _right )
{
    const hb_matching_user_handle_t * user_left = _left;
    const hb_matching_user_handle_t * user_right = _right;

    return user_left->rating - user_right->rating;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_matching_join( hb_matching_handle_t * _matching, const hb_db_client_handle_t * _client, const hb_oid_t * _poid, const char * _name, size_t _namesize, const hb_oid_t * _uoid, uint32_t _rating, const void * _data, size_t _datasize, hb_bool_t * _exist, hb_matching_complete_func_t _complete, void * _ud )
{
    hb_db_collection_handle_t * db_collection_matching;
    if( hb_db_get_collection( _client, "hb", "hb_matching", &db_collection_matching ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "matching", "invalid initialize script: db not found collection '%s'"
            , "hb_matching"
        );

        return HB_FAILURE;
    }

    if( _namesize == HB_UNKNOWN_STRING_SIZE )
    {
        _namesize = strlen( _name );
    }

    hb_db_values_handle_t * find_values;
    if( hb_db_create_values( &find_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_oid_value( find_values, "poid", HB_UNKNOWN_STRING_SIZE, _poid );
    hb_db_make_symbol_value( find_values, "name", HB_UNKNOWN_STRING_SIZE, _name, _namesize );

    hb_oid_t moid;
    hb_bool_t exist;
    if( hb_db_find_oid( db_collection_matching, find_values, &moid, &exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( find_values );

    if( exist == HB_FALSE )
    {
        *_exist = HB_FALSE;

        return HB_SUCCESSFUL;
    }

    *_exist = HB_TRUE;

    hb_matching_room_handle_t * room_found = (hb_matching_room_handle_t *)hb_hashtable_find( _matching->ht, moid.value, sizeof( hb_oid_t ) );

    if( room_found == HB_NULLPTR )
    {
        const char * fields[] = { "matching_count", "matching_dispersion", "join_timeout", "wait_timeout" };

        hb_db_values_handle_t * values;
        if( hb_db_get_values( db_collection_matching, &moid, fields, sizeof( fields ) / sizeof( fields[0] ), &values ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_matching_room_handle_t * new_room = HB_NEW( hb_matching_room_handle_t );

        hb_matching_desc_t desc;
        hb_db_get_uint32_value( values, 0, &desc.matching_count );
        hb_db_get_uint32_value( values, 1, &desc.matching_dispersion );
        hb_db_get_uint32_value( values, 2, &desc.join_timeout );
        hb_db_get_uint32_value( values, 3, &desc.wait_timeout );

        new_room->users = HB_NEWN( hb_matching_user_handle_t, HB_MATCHING_ROOM_USERS_MAX );
        new_room->users_count = 0;
        new_room->users_capacity = HB_MATCHING_ROOM_USERS_MAX;

        if( hb_hashtable_emplace( _matching->ht, moid.value, sizeof( hb_oid_t ), new_room ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_destroy_values( values );

        room_found = new_room;
    }

    for( const hb_matching_user_handle_t * it = room_found->users,
        *it_end = room_found->users + room_found->users_count;
        it != it_end;
        ++it )
    {
        const hb_matching_user_handle_t * user = it;

        if( hb_oid_cmp( &user->uoid, _uoid ) == HB_TRUE )
        {
            return HB_FAILURE;
        }
    }

    if( room_found->users_count >= room_found->users_capacity )
    {
        return HB_FAILURE;
    }

    hb_matching_user_handle_t * new_user = room_found->users + room_found->users_count;

    hb_oid_copy( &new_user->uoid, _uoid );
    new_user->rating = _rating;
    
    if( hb_array_create( _data, _datasize, &new_user->public_data ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    new_user->status = e_hb_matching_user_status_join;

    ++room_found->users_count;

    if( room_found->users_count < room_found->desc.matching_count )
    {
        return HB_SUCCESSFUL;
    }

    qsort( room_found->users, room_found->users_count, sizeof( hb_matching_user_handle_t * ), &__matching_user_cmp );

    uint32_t room_matching_count = room_found->desc.matching_count;
    uint32_t room_matching_dispersion = room_found->desc.matching_dispersion;

    hb_matching_user_handle_t * user_matchings[256];
    uint32_t user_matchings_count = 0;

    for( hb_matching_user_handle_t * it = room_found->users,
        *it_end = room_found->users + room_found->users_count;
        it != it_end;
        ++it )
    {
        hb_matching_user_handle_t * user = it;

        if( user->status != e_hb_matching_user_status_join )
        {
            continue;
        }

        uint32_t user_rating = user->rating;

        if( user_rating > room_matching_dispersion &&
            user_rating - room_matching_dispersion < _rating )
        {
            continue;
        }

        if( user_rating + room_matching_dispersion > _rating )
        {
            break;
        }

        for( hb_matching_user_handle_t * it_dispersion = it + 1,
            *it_dispersion_end = room_found->users + room_found->users_count;
            it_dispersion != it_dispersion_end;
            ++it_dispersion )
        {
            hb_matching_user_handle_t * user_dispersion = it_dispersion;

            if( user_dispersion->status != e_hb_matching_user_status_join )
            {
                continue;
            }

            uint32_t user_dispersion_rating = user_dispersion->rating;

            if( user_dispersion_rating - user_rating > room_matching_dispersion )
            {
                break;
            }

            user_matchings[user_matchings_count] = user_dispersion;
            ++user_matchings_count;

            if( user_matchings_count == room_matching_count )
            {
                break;
            }
        }

        if( user_matchings_count == room_matching_count )
        {
            break;
        }
    }

    if( user_matchings_count == room_matching_count )
    {
        hb_db_values_handle_t * new_world_values;

        if( hb_db_create_values( &new_world_values ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_make_oid_value( new_world_values, "poid", HB_UNKNOWN_STRING_SIZE, _poid );
        hb_db_make_oid_value( new_world_values, "moid", HB_UNKNOWN_STRING_SIZE, &moid );

        hb_oid_t woid;
        if( hb_db_new_document_by_name( _client, "hb_worlds", new_world_values, &woid ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_destroy_values( new_world_values );

        hb_db_values_handle_t * count_world_values;

        if( hb_db_create_values( &count_world_values ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_make_oid_value( count_world_values, "poid", HB_UNKNOWN_STRING_SIZE, _poid );

        hb_pid_t wpid;
        if( hb_db_make_pid_by_name( _client, "hb_worlds", &woid, count_world_values, &wpid ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_db_destroy_values( count_world_values );

        for( hb_matching_user_handle_t ** it_matching = user_matchings,
            **it_matching_end = user_matchings + room_matching_count;
            it_matching != it_matching_end;
            ++it_matching )
        {
            hb_db_values_handle_t * new_avatar_values;

            if( hb_db_create_values( &new_avatar_values ) == HB_FAILURE )
            {
                return HB_FAILURE;
            }

            hb_matching_user_handle_t * matching_user = *it_matching;

            hb_db_make_oid_value( new_avatar_values, "poid", HB_UNKNOWN_STRING_SIZE, _poid );
            hb_db_make_oid_value( new_avatar_values, "woid", HB_UNKNOWN_STRING_SIZE, &woid );
            hb_db_make_oid_value( new_avatar_values, "uoid", HB_UNKNOWN_STRING_SIZE, &matching_user->uoid );
            hb_db_make_int32_value( new_avatar_values, "rating", HB_UNKNOWN_STRING_SIZE, matching_user->rating );

            hb_oid_t aoid;
            if( hb_db_new_document_by_name( _client, "hb_avatars", new_avatar_values, &aoid ) == HB_FAILURE )
            {
                return HB_FAILURE;
            }

            hb_db_destroy_values( new_avatar_values );

            hb_db_values_handle_t * count_avatar_values;

            if( hb_db_create_values( &count_avatar_values ) == HB_FAILURE )
            {
                return HB_FAILURE;
            }

            hb_db_make_oid_value( count_avatar_values, "poid", HB_UNKNOWN_STRING_SIZE, _poid );
            hb_db_make_oid_value( count_avatar_values, "woid", HB_UNKNOWN_STRING_SIZE, &woid );

            if( hb_db_make_pid_by_name( _client, "hb_avatars", &aoid, count_avatar_values, &matching_user->apid ) == HB_FAILURE )
            {
                return HB_FAILURE;
            }

            hb_db_destroy_values( count_avatar_values );
        }

        const char * fields[] = { "public_data" };

        hb_db_values_handle_t * values;

        if( hb_db_get_values( db_collection_matching, &moid, fields, 1, &values ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_matching_complete_desc_t desc;
        desc.name = _name;
        desc.wpid = wpid;
        desc.users = user_matchings;
        desc.users_count = room_matching_count;
        hb_db_get_symbol_value( values, 0, &desc.data, &desc.data_size );
        desc.ud = _ud;

        hb_result_t result = (*_complete)(&desc);

        hb_db_destroy_values( values );

        if( result == HB_FAILURE )
        {
            for( hb_matching_user_handle_t ** it_matching = user_matchings,
                **it_matching_end = user_matchings + room_matching_count;
                it_matching != it_matching_end;
                ++it_matching )
            {
                hb_matching_user_handle_t * matching_user = *it_matching;

                matching_user->status = e_hb_matching_user_status_join;
            }

            return HB_FAILURE;
        }
        else
        {
            for( hb_matching_user_handle_t ** it_matching = user_matchings,
                **it_matching_end = user_matchings + room_matching_count;
                it_matching != it_matching_end;
                ++it_matching )
            {
                hb_matching_user_handle_t * matching_user = *it_matching;

                matching_user->status = e_hb_matching_user_status_wait;
            }
        }
    }

    hb_db_destroy_collection( db_collection_matching );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_matching_found( hb_matching_handle_t * _matching, const hb_db_client_handle_t * _client, const hb_oid_t * _poid, const char * _name, size_t _namesize, const hb_oid_t * _uoid, hb_bool_t * _exist, hb_pid_t * _apid )
{
    hb_db_collection_handle_t * db_collection_matching;
    if( hb_db_get_collection( _client, "hb", "hb_matching", &db_collection_matching ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "matching", "invalid initialize script: db not found collection '%s'"
            , "hb_matching"
        );

        return HB_FAILURE;
    }

    if( _namesize == HB_UNKNOWN_STRING_SIZE )
    {
        _namesize = strlen( _name );
    }

    hb_db_values_handle_t * find_values;

    if( hb_db_create_values( &find_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_oid_value( find_values, "poid", HB_UNKNOWN_STRING_SIZE, _poid );
    hb_db_make_symbol_value( find_values, "name", HB_UNKNOWN_STRING_SIZE, _name, _namesize );

    hb_oid_t moid;
    hb_bool_t exist;
    if( hb_db_find_oid( db_collection_matching, find_values, &moid, &exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( find_values );

    if( exist == HB_FALSE )
    {
        *_exist = HB_FALSE;

        return HB_SUCCESSFUL;
    }

    hb_matching_room_handle_t * room_found = (hb_matching_room_handle_t *)hb_hashtable_find( _matching->ht, &moid, sizeof( hb_oid_t ) );

    if( room_found == HB_NULLPTR )
    {
        *_exist = HB_FALSE;

        return HB_SUCCESSFUL;
    }

    for( const hb_matching_user_handle_t * it = room_found->users,
        *it_end = room_found->users + room_found->users_count;
        it != it_end;
        ++it )
    {
        const hb_matching_user_handle_t * user = it;

        if( hb_oid_cmp( &user->uoid, _uoid ) == HB_FALSE )
        {
            continue;
        }

        *_exist = HB_TRUE;

        *_apid = user->apid;
    }

    *_exist = HB_FALSE;

    hb_db_destroy_collection( db_collection_matching );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_matching_ready( hb_matching_handle_t * _matching, const hb_db_client_handle_t * _client, const hb_oid_t * _poid, const char * _name, size_t _namesize, const hb_oid_t * _uoid, hb_pid_t _apid, hb_bool_t * _exist )
{
    hb_db_collection_handle_t * db_collection_matching;
    if( hb_db_get_collection( _client, "hb", "hb_matching", &db_collection_matching ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "matching", "invalid initialize script: db not found collection '%s'"
            , "hb_matching"
        );

        return HB_FAILURE;
    }

    if( _namesize == HB_UNKNOWN_STRING_SIZE )
    {
        _namesize = strlen( _name );
    }

    hb_db_values_handle_t * find_values;

    if( hb_db_create_values( &find_values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_oid_value( find_values, "poid", HB_UNKNOWN_STRING_SIZE, _poid );
    hb_db_make_symbol_value( find_values, "name", HB_UNKNOWN_STRING_SIZE, _name, _namesize );

    hb_oid_t moid;
    hb_bool_t exist;
    if( hb_db_find_oid( db_collection_matching, find_values, &moid, &exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( find_values );

    if( exist == HB_FALSE )
    {
        *_exist = HB_FALSE;

        return HB_SUCCESSFUL;
    }

    hb_matching_room_handle_t * room_found = (hb_matching_room_handle_t *)hb_hashtable_find( _matching->ht, moid.value, sizeof( hb_oid_t ) );

    if( room_found == HB_NULLPTR )
    {
        *_exist = HB_FALSE;

        return HB_SUCCESSFUL;
    }

    for( hb_matching_user_handle_t * it = room_found->users,
        *it_end = room_found->users + room_found->users_count;
        it != it_end;
        ++it )
    {
        hb_matching_user_handle_t * user = it;

        if( user->apid != _apid )
        {
            continue;
        }

        if( hb_oid_cmp( &user->uoid, _uoid ) == HB_FALSE )
        {
            *_exist = HB_FALSE;

            return HB_SUCCESSFUL;
        }

        if( user->status == e_hb_matching_user_status_wait )
        {
            *_exist = HB_FALSE;

            return HB_SUCCESSFUL;
        }

        *_exist = HB_TRUE;

        user->status = e_hb_matching_user_status_ready;

        return HB_SUCCESSFUL;
    }

    *_exist = HB_FALSE;

    hb_db_destroy_collection( db_collection_matching );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_pid_t hb_matching_user_get_apid( const hb_matching_user_handle_t * _user )
{
    return _user->apid;
}
//////////////////////////////////////////////////////////////////////////
int32_t hb_matching_user_get_rating( const hb_matching_user_handle_t * _user )
{
    return _user->rating;
}
//////////////////////////////////////////////////////////////////////////
const void * hb_matching_user_get_public_data( const hb_matching_user_handle_t * _user, size_t * _size )
{
    const void * data = hb_array_data( _user->public_data, _size );

    return data;
}
