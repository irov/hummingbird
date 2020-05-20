#include "hb_grid_process_getleaderboard.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_leaderboard/hb_leaderboard.h"

hb_result_t hb_grid_process_getleaderboard( hb_grid_process_handle_t * _process, const hb_grid_process_getleaderboard_in_data_t * _in, hb_grid_process_getleaderboard_out_data_t * _out )
{
    HB_UNUSED( _out );

    if( hb_cache_expire_value( _process->cache, _in->token.value, sizeof( _in->token ), 1800 ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_user_token_t token_handle;
    if( hb_cache_get_value( _process->cache, _in->token.value, sizeof( _in->token ), &token_handle, sizeof( token_handle ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    uint32_t descs_count;
    hb_oid_t oids[16];
    uint32_t scores[16];
    if( hb_leaderboard_get( _process->cache, &token_handle.poid, _in->begin, _in->end, oids, scores, &descs_count ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    const char * fields[] = { "info_nickname" };

    hb_db_values_handle_t * values;
    hb_db_gets_values_by_name( _process->db_client, "hb_users", oids, descs_count, fields, sizeof( fields ) / sizeof( fields[0] ), &values );

    for( uint32_t index = 0; index != descs_count; ++index )
    {
        _out->descs[index].score = scores[index];

        size_t nickname_len;
        const char * nickname;
        if( hb_db_get_symbol_value( values, index, &nickname, &nickname_len ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        memcpy( _out->descs[index].nickname, nickname, nickname_len );
        _out->descs[index].nickname[nickname_len] = '\0';
    }
    
    _out->descs_count = descs_count;

    hb_db_destroy_values( values );

    return HB_SUCCESSFUL;
}