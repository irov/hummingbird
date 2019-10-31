#ifndef HB_LOG_TCP_H_
#define HB_LOG_TCP_H_

#include "hb_log/hb_log.h"

hb_result_t hb_log_tcp_initialize( const char * _url, uint16_t _port );
void hb_log_tcp_finalize();

#endif
