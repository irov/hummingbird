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
    hb_uid_t oids[16] = {0};
    uint32_t scores[16] = {0};
    if( hb_leaderboard_get_global( _process->cache, token_handle.puid, _in->begin, _in->end, oids, scores, &descs_count ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    const char * fields[] = { "_id", "info_nickname" };

    hb_db_values_handle_t * values;
    if( hb_db_gets_values_by_name( _process->db_client, "hb_users", oids, descs_count, fields, sizeof( fields ) / sizeof( fields[0] ), &values ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    for( uint32_t index = 0; index != descs_count; ++index )
    {
        _out->descs[index].score = scores[index];

        hb_uid_t uuid;
        if( hb_db_get_uid_value( values, index * 2 + 0, &uuid ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        _out->descs[index].uuid = uuid;

        size_t nickname_len;
        const char * nickname;
        if( hb_db_get_string_value( values, index * 2 + 1, &nickname, &nickname_len ) == HB_FAILURE )
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