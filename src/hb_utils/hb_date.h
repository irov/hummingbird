#ifndef HB_DATE_H_
#define HB_DATE_H_

#include "hb_config/hb_config.h"

typedef struct hb_date_t
{
    uint32_t year;
    uint32_t mon;
    uint32_t mday;
    uint32_t hour;
    uint32_t min;
    uint32_t sec;
} hb_date_t;

void hb_date( hb_date_t * _date );

#endif
