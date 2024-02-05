#ifndef HB_GETOPT_H_
#define HB_GETOPT_H_

#include "hb_config/hb_config.h"

hb_result_t hb_getopt( int argc, char * argv[], const char * _name, const char ** _value );
hb_result_t hb_getopti( int argc, char * argv[], const char * _name, int64_t * const _value );

#endif