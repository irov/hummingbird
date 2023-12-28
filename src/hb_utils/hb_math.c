#include "hb_math.h"

//////////////////////////////////////////////////////////////////////////
uint32_t hb_nearest_pow2( uint32_t _value )
{
    if( _value == 0 )
    {
        return 1;
    }

    uint32_t v = _value;

    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;

    v++;

    return v;
}
//////////////////////////////////////////////////////////////////////////
uint32_t hb_log2( uint32_t _value )
{
    const uint32_t tab32[32] = {
        0, 9, 1, 10, 13, 21, 2, 29,
        11, 14, 16, 18, 22, 25, 3, 30,
        8, 12, 20, 28, 15, 17, 24, 7,
        19, 27, 23, 6, 26, 5, 4, 31
    };

    uint32_t v = _value;

    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;

    uint32_t index = (v * 0x07C4ACDD) >> 27;

    uint32_t l2 = tab32[index];

    return l2;
}
//////////////////////////////////////////////////////////////////////////
uint32_t hb_pow2( uint32_t _exponent )
{
    uint32_t v = 1 << _exponent;

    return v;
}
//////////////////////////////////////////////////////////////////////////
uint32_t hb_clamp( uint32_t _l, uint32_t _r, uint32_t _value )
{
    if( _value < _l )
    {
        return _l;
    }

    if( _value > _r )
    {
        return _r;
    }

    return _value;
}
//////////////////////////////////////////////////////////////////////////
hb_size_t hb_clampz( hb_size_t _l, hb_size_t _r, hb_size_t _value )
{
    if( _value < _l )
    {
        return _l;
    }

    if( _value > _r )
    {
        return _r;
    }

    return _value;
}
//////////////////////////////////////////////////////////////////////////