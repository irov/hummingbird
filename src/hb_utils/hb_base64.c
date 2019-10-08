#include "hb_base64.h"

//////////////////////////////////////////////////////////////////////////
int hb_base64_decode( const uint8_t * _data, size_t _datasize, char * _base64, size_t _capacity, size_t * _base64size )
{
    size_t base64size = (_datasize << 1) + 1;

    if( base64size > _capacity )
    {
        return 0;
    }

    const char base64hex[] = {"0123456789abcdef"};

    for( size_t index = 0; index != _datasize; ++index )
    {
        _base64[(index << 1) + 0] = base64hex[(_data[index] >> 4) & 0xf];
        _base64[(index << 1) + 1] = base64hex[(_data[index] >> 0) & 0xf];
    }    

    _base64[base64size] = '\0';

    if( _base64size != HB_NULLPTR )
    {
        *_base64size = base64size;
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_base64_encode( const char * _base64, size_t _base64size, uint8_t * _data, size_t _capacity, size_t * _datasize )
{
    size_t datasize = _base64size >> 1;

    if( datasize > _capacity )
    {
        return 0;
    }

    const uint8_t base64_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //15
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //31
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //47
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, //63
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //95
        0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, //111
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

    for( size_t index = 0; index != _base64size; ++index )
    {
        uint8_t data0 = base64_table[_base64[(index << 1) + 0]];
        uint8_t data1 = base64_table[_base64[(index << 1) + 1]];

        _data[index] = (data0 << 4) + data1;
    }

    if( _datasize != HB_NULLPTR )
    {
        *_datasize = datasize;
    }

    return 1;
}