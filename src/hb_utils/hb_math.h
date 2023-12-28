#ifndef HB_MATH_H_
#define HB_MATH_H_

#include "hb_config/hb_config.h"

uint32_t hb_nearest_pow2( uint32_t _value );
uint32_t hb_log2( uint32_t _value );
uint32_t hb_pow2( uint32_t _exponent );
uint32_t hb_clamp( uint32_t _l, uint32_t _r, uint32_t _value );
hb_size_t hb_clampz( hb_size_t _l, hb_size_t _r, hb_size_t _value );

#endif
