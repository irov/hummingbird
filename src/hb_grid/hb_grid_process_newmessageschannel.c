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

hb_result_t hb_grid_process_newmessageschannel( hb_grid_process_handle_t * _process, const hb_grid_process_newmessageschannel_in_data_t * _in, hb_grid_process_newmessageschannel_out_data_t * const _out )
{
    hb_uid_t cuid;
    if( hb_messages_new_channel( _process->messages, _process->db_client, _in->project_uid, _in->messageschannel_maxpost, &cuid ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    _out->cuid = cuid;

    return HB_SUCCESSFUL;
}