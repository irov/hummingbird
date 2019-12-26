#ifndef HB_OID_H_
#define HB_OID_H_

#include "hb_config/hb_config.h"

void hb_oid_clear( hb_oid_t _oid );
void hb_oid_copy( hb_oid_t _dst, const hb_oid_t _src );
void hb_oid_make( const char * _str, hb_oid_t * _oid );
hb_bool_t hb_oid_cmp( const hb_oid_t _dst, const hb_oid_t _src );
void hb_oid16_make( const char * _str, hb_oid16_t * _base16 );
void hb_oid_base16_encode( hb_oid_t _oid, hb_oid16_t * _base16 );
void hb_oid_base16_decode( hb_oid16_t _base16, hb_oid_t * _oid );

#endif