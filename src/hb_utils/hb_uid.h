#ifndef HB_UID_H_
#define HB_UID_H_

#include "hb_config/hb_config.h"

hb_result_t hb_uid_base16_encode( const hb_uid_t _uid, hb_uid16_t * _base16 );
hb_result_t hb_uid_base16_decode( const hb_uid16_t * _base16, hb_uid_t * _uid );

#endif