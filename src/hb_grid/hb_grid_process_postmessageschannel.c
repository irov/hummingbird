#include "hb_grid_process_postmessageschannel.h"

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

hb_result_t hb_grid_process_postmessageschannel( hb_grid_process_handle_t * _process, const hb_grid_process_postmessageschannel_in_data_t * _in, hb_grid_process_postmessageschannel_out_data_t * _out )
{
    hb_messages_post_t post;
    post.uuid = _in->uuid;
    post.message = _in->message;
    post.metainfo = _in->metainfo;

    uint32_t postid;
    hb_error_code_t code;
    if( hb_messages_channel_new_post( _process->messages, _process->db_client, _in->puid, _in->cuid, &post, &postid, &code ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    _out->postid = postid;
    _out->code = code;

    return HB_SUCCESSFUL;
}