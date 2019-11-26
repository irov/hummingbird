#ifndef HB_PROCESS_H_
#define HB_PROCESS_H_

#include "hb_config/hb_config.h"
#include "hb_sharedmemory/hb_sharedmemory.h"

hb_result_t hb_process_run( const char * _name, hb_sharedmemory_handle_t * _handle, hb_bool_t * _successful );

#endif
