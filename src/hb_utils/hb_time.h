#ifndef HB_TIME_H_
#define HB_TIME_H_

#include "hb_config/hb_config.h"

#define HB_TIME_SECONDS_IN_WEEK 604800
#define HB_TIME_SECONDS_IN_DAY 86400
#define HB_TIME_SECONDS_IN_HOUR 3600
#define HB_TIME_SECONDS_IN_MINUTE 60

typedef uint64_t hb_time_t;

void hb_time( hb_time_t * _time );

#endif
