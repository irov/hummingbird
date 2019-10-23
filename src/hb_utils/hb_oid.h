#ifndef HB_OID_H_
#define HB_OID_H_

#include "hb_config/hb_config.h"

typedef uint8_t hb_oid_t[12];

void hb_oid_copy( hb_oid_t _dst, const hb_oid_t _src );

#endif