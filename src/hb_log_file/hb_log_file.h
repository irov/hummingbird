#ifndef HB_LOG_FILE_H_
#define HB_LOG_FILE_H_

#include "hb_log/hb_log.h"

hb_result_t hb_log_file_initialize( const char * _path );
void hb_log_file_finalize();

#endif
