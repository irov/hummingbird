#include "hb_grid_process_neweconomicsrecords.h"

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

hb_result_t hb_grid_process_neweconomicsrecords( hb_grid_process_handle_t * _process, const hb_grid_process_neweconomicsrecords_in_data_t * _in, hb_grid_process_neweconomicsrecords_out_data_t * _out )
{
    HB_UNUSED( _out );

    if( hb_economics_new_records( _process->economics, _process->db_client, _in->puid, _in->records_source, _in->records_source_size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}