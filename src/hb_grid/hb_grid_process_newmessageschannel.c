#include "hb_grid_process_newmessageschannel.h"

#include "hb_log/hb_log.h"
#include "hb_log_tcp/hb_log_tcp.h"
#include "hb_messages/hb_messages.h"
#include "hb_db/hb_db.h"
#include "hb_cache/hb_cache.h"
#include "hb_script/hb_script.h"
#include "hb_script/hb_script_compiler.h"
#include "hb_storage/hb_storage.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_httpopt.h"
#include "hb_utils/hb_memmem.h"

hb_result_t hb_grid_process_newmessageschannel( hb_grid_process_handle_t * _process, const hb_grid_process_newmessageschannel_in_data_t * _in, hb_grid_process_newmessageschannel_out_data_t * _out )
{
    HB_UNUSED( _process );

    if( hb_cache_expire_value( _process->cache, _in->token.value, sizeof( _in->token ), 1800 ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_account_token_t token_handle;
    if( hb_cache_get_value( _process->cache, _in->token.value, sizeof( _in->token ), &token_handle, sizeof( token_handle ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_uid_t muid;
    if( hb_messages_new_channel( _process->messages, _process->db_client, _in->puid, _in->maxpost, &muid ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    _out->muid = muid;

    return HB_SUCCESSFUL;
}