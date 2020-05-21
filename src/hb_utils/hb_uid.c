#include "hb_uid.h"

#include "hb_utils/hb_base16.h"

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_uid_base16_encode( const hb_uid_t _uid, hb_uid16_t * _base16 )
{
    hb_result_t result = hb_base16_encode( &_uid, sizeof( hb_uid_t ), _base16->value, sizeof( hb_uid16_t ), HB_NULLPTR );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_uid_base16_decode( const hb_uid16_t * _base16, hb_uid_t * _uid )
{
    hb_result_t result = hb_base16_decode( _base16->value, sizeof( hb_uid16_t ), _uid, sizeof( hb_uid_t ), HB_NULLPTR );

    return result;
}