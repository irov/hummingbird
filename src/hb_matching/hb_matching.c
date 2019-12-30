#include "hb_matching.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"

#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_oid.h"

#include <string.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_matching_initialize( uint32_t _count, hb_matching_t ** _matching )
{
    hb_matching_t * matching = HB_NEW( hb_matching_t );

    hb_hashtable_t * ht;
    hb_hashtable_create( _count, &ht );

    matching->ht = ht;

    *_matching = matching;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_matching_finalize( hb_matching_t * _matching )
{
    hb_hashtable_destroy( _matching->ht );
    HB_DELETE( _matching );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_matching_create( hb_matching_t * _matching, hb_oid_t _poid, const char * _name, size_t _namesize, uint32_t _count, uint32_t _dispersion, const void * _data, size_t _datasize, hb_bool_t * _exist )
{
    hb_db_collection_handle_t * db_collection_matching;
    if( hb_db_get_collection( "hb", "hb_matching", &db_collection_matching ) == HB_FAILURE )
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

    hb_db_value_handle_t find_values[2];
    hb_db_make_oid_value( "poid", HB_UNKNOWN_STRING_SIZE, _poid, find_values + 0 );
    hb_db_make_symbol_value( "name", HB_UNKNOWN_STRING_SIZE, _name, _namesize, find_values + 1 );

    hb_bool_t exist;
    if( hb_db_find_oid( db_collection_matching, find_values, 2, HB_NULLPTR, &exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( exist == HB_TRUE )
    {
        *_exist = HB_TRUE;

        return HB_SUCCESSFUL;
    }

    *_exist = HB_FALSE;

    hb_db_value_handle_t room_values[5];
    hb_db_make_oid_value( "poid", HB_UNKNOWN_STRING_SIZE, _poid, room_values + 0 );
    hb_db_make_symbol_value( "name", HB_UNKNOWN_STRING_SIZE, _name, _namesize, room_values + 1 );
    hb_db_make_int32_value( "count", HB_UNKNOWN_STRING_SIZE, (int32_t)_count, room_values + 2 );
    hb_db_make_int32_value( "dispersion", HB_UNKNOWN_STRING_SIZE, (int32_t)_dispersion, room_values + 3 );
    hb_db_make_symbol_value( "public_data", HB_UNKNOWN_STRING_SIZE, _data, _datasize, room_values + 4 );

    hb_oid_t moid;
    if( hb_db_new_document( db_collection_matching, room_values, 5, &moid ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_matching_room_t * new_room = HB_NEW( hb_matching_room_t );

    new_room->count = _count;
    new_room->dispersion = _dispersion;

    new_room->users = HB_NEWN( hb_matching_user_t, 64 );
    new_room->users_count = 0;
    new_room->users_capacity = 64;

    hb_hashtable_emplace( _matching->ht, moid, sizeof( hb_oid_t ), new_room );

    hb_db_destroy_collection( db_collection_matching );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static int32_t __matching_user_cmp( const void * _left, const void * _right )
{
    const hb_matching_user_t * user_left = _left;
    const hb_matching_user_t * user_right = _right;

    return user_left->rating - user_right->rating;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_matching_join( hb_matching_t * _matching, hb_oid_t _poid, const char * _name, size_t _namesize, hb_oid_t _uoid, int32_t _rating, hb_bool_t * _exist, hb_matching_complete_t _complete, void * _ud )
{
    hb_db_collection_handle_t * db_collection_matching;
    if( hb_db_get_collection( "hb", "hb_matching", &db_collection_matching ) == HB_FAILURE )
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

    hb_db_value_handle_t find_values[2];
    hb_db_make_oid_value( "poid", HB_UNKNOWN_STRING_SIZE, _poid, find_values + 0 );
    hb_db_make_symbol_value( "name", HB_UNKNOWN_STRING_SIZE, _name, _namesize, find_values + 1 );

    hb_oid_t moid;
    hb_bool_t exist;
    if( hb_db_find_oid( db_collection_matching, find_values, 2, &moid, &exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( exist == HB_FALSE )
    {
        *_exist = HB_FALSE;

        return HB_SUCCESSFUL;
    }

    *_exist = HB_TRUE;

    hb_matching_room_t * room_found = (hb_matching_room_t *)hb_hashtable_find( _matching->ht, moid, sizeof( hb_oid_t ) );

    if( room_found == HB_NULLPTR )
    {
        const char * fields[] = {"count", "dispersion"};

        hb_db_value_handle_t values[2];
        if( hb_db_get_values( db_collection_matching, moid, fields, values, 2 ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_matching_room_t * new_room = HB_NEW( hb_matching_room_t );

        new_room->count = values[0].u.i32;
        new_room->dispersion = values[1].u.i32;

        new_room->users = HB_NEWN( hb_matching_user_t, 64 );
        new_room->users_count = 0;
        new_room->users_capacity = 64;

        hb_hashtable_emplace( _matching->ht, moid, sizeof( hb_oid_t ), new_room );

        hb_db_destroy_values( values, 2 );

        room_found = new_room;
    }

    for( const hb_matching_user_t * it = room_found->users,
        *it_end = room_found->users + room_found->users_count;
        it != it_end;
        ++it )
    {
        const hb_matching_user_t * user = it;

        if( hb_oid_cmp( user->uoid, _uoid ) == HB_TRUE )
        {
            return HB_FAILURE;
        }
    }

    if( room_found->users_count >= room_found->users_capacity )
    {
        return HB_FAILURE;
    }

    hb_matching_user_t * new_user = room_found->users + room_found->users_count;

    hb_oid_copy( new_user->uoid, _uoid );
    new_user->rating = _rating;

    ++room_found->users_count;

    qsort( room_found->users, room_found->users_count, sizeof( hb_matching_user_t * ), &__matching_user_cmp );

    hb_matching_user_t * user_matching = HB_NULLPTR;

    hb_bool_t successful = HB_TRUE;

    for( hb_matching_user_t * it = room_found->users,
        *it_end = room_found->users + room_found->users_count - room_found->count;
        it != it_end;
        ++it )
    {
        hb_matching_user_t * user = it;

        int32_t user_rating = user->rating;

        successful = HB_TRUE;

        for( hb_matching_user_t * it_dispersion = it,
            *it_dispersion_end = it + room_found->count;
            it_dispersion != it_dispersion_end;
            ++it_dispersion )
        {
            hb_matching_user_t * user_dispersion = it_dispersion;

            int32_t user_dispersion_rating = user_dispersion->rating;

            if( user_dispersion_rating - user_rating > room_found->dispersion )
            {
                successful = HB_FALSE;

                break;
            }
        }

        if( successful == HB_TRUE )
        {
            user_matching = user;

            break;
        }
    }

    if( successful == HB_TRUE )
    {
        hb_db_value_handle_t new_world_values[2];
        hb_db_make_oid_value( "moid", HB_UNKNOWN_STRING_SIZE, moid, new_world_values + 0 );
        hb_db_make_oid_value( "poid", HB_UNKNOWN_STRING_SIZE, _poid, new_world_values + 1 );

        hb_oid_t woid;
        if( hb_db_new_document_by_name( "hb_worlds", new_world_values, 2, &woid ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        for( hb_matching_user_t * it_matching = user_matching,
            *it_matching_end = user_matching + room_found->count;
            it_matching != it_matching_end;
            ++it_matching )
        {
            hb_db_value_handle_t new_avatar_values[3];
            hb_db_make_oid_value( "woid", HB_UNKNOWN_STRING_SIZE, woid, new_avatar_values + 0 );
            hb_db_make_oid_value( "uoid", HB_UNKNOWN_STRING_SIZE, it_matching->uoid, new_avatar_values + 1 );
            hb_db_make_int32_value( "rating", HB_UNKNOWN_STRING_SIZE, it_matching->rating, new_avatar_values + 2 );

            if( hb_db_new_document_by_name( "hb_avatars", new_avatar_values, 3, &it_matching->aoid ) == HB_FAILURE )
            {
                return HB_FAILURE;
            }
        }

        const char * fields[] = { "public_data" };

        hb_db_value_handle_t values[1];
        if( hb_db_get_values( db_collection_matching, moid, fields, values, 1 ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_result_t result = (*_complete)(user_matching, room_found->count, values[0].u.symbol.buffer, values[0].u.symbol.length, _ud);

        hb_db_destroy_values( values, 1 );

        for( hb_matching_user_t * it_matching = user_matching,
            *it_matching_end = user_matching + room_found->count;
            it_matching != it_matching_end;
            ++it_matching )
        {
            --room_found->users_count;
            *it_matching = *(room_found->users + room_found->users_count);
        }

        if( result == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }

    hb_db_destroy_collection( db_collection_matching );

    return HB_SUCCESSFUL;
}