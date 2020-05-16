#include "hb_oid.h"

#include "hb_utils/hb_base16.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
void hb_oid_clear( hb_oid_t _oid )
{
    memset( _oid, 0x00, sizeof( hb_oid_t ) );
}
//////////////////////////////////////////////////////////////////////////
void hb_oid_copy( hb_oid_t _dst, const hb_oid_t _src )
{
    memcpy( _dst, _src, sizeof( hb_oid_t ) );
}
//////////////////////////////////////////////////////////////////////////
void hb_oid_make( const char * _str, hb_oid_t * _oid )
{
    hb_base16_decode( _str, sizeof( hb_oid16_t ), *_oid, sizeof( hb_oid_t ), HB_NULLPTR );
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_oid_cmp( const hb_oid_t _dst, const hb_oid_t _src )
{
    if( memcmp( _dst, _src, sizeof( hb_oid_t ) ) != 0 )
    {
        return HB_FALSE;
    }

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////
void hb_oid16_make( const char * _str, hb_oid16_t * _base16 )
{
    memcpy( *_base16, _str, sizeof( hb_oid16_t ) );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_oid_base16_encode( const hb_oid_t _oid, hb_oid16_t * _base16 )
{
    hb_result_t result = hb_base16_encode( _oid, sizeof( hb_oid_t ), *_base16, sizeof( hb_oid16_t ), HB_NULLPTR );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_oid_base16_decode( const hb_oid16_t _base16, hb_oid_t * _oid )
{
    hb_result_t result = hb_base16_decode( _base16, sizeof( hb_oid16_t ), *_oid, sizeof( hb_oid_t ), HB_NULLPTR );

    return result;
}