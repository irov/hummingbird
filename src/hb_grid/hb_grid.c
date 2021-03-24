#include "hb_grid.h"

#include "hb_mutex/hb_mutex.h"

//////////////////////////////////////////////////////////////////////////
void hb_grid_process_lock( hb_grid_process_handle_t * _process, hb_uid_t _uuid )
{
    hb_grid_mutex_handle_t * mutex_handle = _process->mutex_handles + _uuid % _process->mutex_count;
    hb_mutex_lock( mutex_handle->mutex );
}
//////////////////////////////////////////////////////////////////////////
void hb_grid_process_unlock( hb_grid_process_handle_t * _process, hb_uid_t _uuid )
{
    hb_grid_mutex_handle_t * mutex_handle = _process->mutex_handles + _uuid % _process->mutex_count;
    hb_mutex_unlock( mutex_handle->mutex );
}
//////////////////////////////////////////////////////////////////////////
