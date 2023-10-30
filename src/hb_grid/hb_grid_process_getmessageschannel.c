#include "hb_grid_process_getmessageschannel.h"

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

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_process_getmessageschannel( hb_grid_process_handle_t * _process, const hb_grid_process_getmessageschannel_in_data_t * _in, hb_grid_process_getmessageschannel_out_data_t * const _out )
{
    if( hb_messages_channel_get_posts( _process->messages, _in->project_uid, _in->messageschannel_uid, _in->messageschannel_postid, _out->posts, 256, &_out->posts_count, &_out->code ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}