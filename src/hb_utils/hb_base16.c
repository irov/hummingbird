#include "hb_base16.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
static const uint8_t base16_decode_table[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //15
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //31
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //47
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //63
    0, 0, 0, 1, 2, 0, 0, 3, 4, 0, 5, 6, 7, 8, 9, 0, //79
    10, 11, 12, 13, 14, 0, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, //95
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //111
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //127
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //143
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //159
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //175
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //191
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //207
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //223
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //239
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //255
};
//////////////////////////////////////////////////////////////////////////
size_t hb_base16_encode_size( size_t _size )
{
    size_t totalsize = _size * 2;

    return totalsize;
}
//////////////////////////////////////////////////////////////////////////
size_t hb_base16_decode_size( size_t _size )
{
    size_t totalsize = _size / 2;

    return totalsize;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_base16_encode( const void * _data, size_t _size, char * _base16, size_t _capacity, size_t * _outsize )
{
    size_t totalsize = hb_base16_encode_size( _size );

    if( totalsize > _capacity )
    {
        return HB_FAILURE;
    }

    const char base16_encode_table[] = { 
        'B', 'C', 'D', 'G', 
        'H', 'J', 'K', 'L',
        'M', 'N', 'P', 'Q', 
        'R', 'S', 'T', 'V' };

    const uint8_t * data = (const uint8_t *)_data;

    for( size_t i = 0, j = 0; i != _size;)
    {
        uint8_t v = data[i++];

        uint8_t a = (v >> 0) & 0xf;
        uint8_t b = (v >> 4) & 0xf;

        _base16[j++] = base16_encode_table[a];
        _base16[j++] = base16_encode_table[b];
    }

    if( _outsize != HB_NULLPTR )
    {
        *_outsize = totalsize;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_base16_decode( const char * _base16, size_t _size, void * _data, size_t _capacity, size_t * _outsize )
{
    if( _size == ~0U )
    {
        _size = strlen( _base16 );
    }

    size_t totalsize = hb_base16_decode_size( _size );

    if( totalsize > _capacity )
    {
        return HB_FAILURE;
    }

    uint8_t * data = (uint8_t *)_data;

    for( uint32_t i = 0, j = 0; i != _size;)
    {
        uint8_t a = base16_decode_table[_base16[i++]];
        uint8_t b = base16_decode_table[_base16[i++]];

        data[j++] = (a << 0) | (b << 4);
    }

    if( _outsize != HB_NULLPTR )
    {
        *_outsize = totalsize;
    }

    return HB_SUCCESSFUL;
}