#ifndef HB_PID_H_
#define HB_PID_H_

#include "hb_config/hb_config.h"

hb_result_t hb_pid_base16_encode( const hb_pid_t _pid, hb_pid16_t * _base16 );
hb_result_t hb_pid_base16_decode( const hb_pid16_t _base16, hb_pid_t * _pid );

#endif