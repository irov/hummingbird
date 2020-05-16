#include "hb_pid.h"

#include "hb_utils/hb_base16.h"

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_pid_base16_encode( const hb_pid_t _pid, hb_pid16_t * _base16 )
{
    hb_result_t result = hb_base16_encode( &_pid, sizeof( hb_pid_t ), *_base16, sizeof( hb_pid16_t ), HB_NULLPTR );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_pid_base16_decode( const hb_pid16_t _base16, hb_pid_t * _pid )
{
    hb_result_t result = hb_base16_decode( _base16, sizeof( hb_pid16_t ), _pid, sizeof( hb_pid_t ), HB_NULLPTR );

    return result;
}