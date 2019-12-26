#include "hb_matching.h"

#include "hb_db/hb_db.h"

#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_oid.h"

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
hb_result_t hb_matching_create( hb_matching_t * _matching, hb_oid16_t _moid16, hb_oid16_t _uoid16 )
{
    hb_matching_room_t * room_found = (hb_matching_room_t *)hb_hashtable_find( _matching->ht, _moid16, sizeof( hb_oid16_t ) );

    if( room_found == HB_NULLPTR )
    {
        hb_db_collection_handle_t * db_collection_matching;
        if( hb_db_get_collection( "hb", "hb_matching", &db_collection_matching ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        hb_oid_t moid;
        hb_oid_base16_decode( _moid16, &moid );

        const char * fields[] = { "count", "dispersion" };

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

        hb_hashtable_emplace( _matching->ht, _moid16, sizeof( hb_oid16_t ), new_room );

        hb_db_destroy_collection( db_collection_matching );
    }

    HB_UNUSED( _uoid16 );

    return HB_SUCCESSFUL;
}