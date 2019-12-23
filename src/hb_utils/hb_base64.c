#include "hb_base64.h"

//////////////////////////////////////////////////////////////////////////
static const hb_byte_t base64_decode_table[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //15
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //31
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 63, 0, 0, 0, 0, //47
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, //63
    0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, //79
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0, //95
    0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, //111
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0, 0, 0, 0, //127
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
size_t hb_base64_encode_size( size_t _size )
{
    size_t totalsize = 4 * ((_size + 2) / 3);

    const uint32_t mod_table[] = {0, 2, 1};

    const uint32_t mod = mod_table[_size % 3];

    totalsize -= mod;

    return totalsize;
}
//////////////////////////////////////////////////////////////////////////
size_t hb_base64_decode_size( size_t _size )
{
    size_t totalsize = (_size + 3) / 4 * 3;

    const uint32_t mod_table[] = {0, 3, 2, 1};

    const uint32_t mod = mod_table[_size % 4];

    totalsize -= mod;

    return totalsize;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_base64_encode( const void * _data, size_t _size, char * _base64, size_t _capacity, size_t * _outsize )
{
    size_t totalsize = hb_base64_encode_size( _size );

    if( totalsize > _capacity )
    {
        return HB_FAILURE;
    }

    const char base64_encode_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9', '*', '+'};

    const hb_byte_t * data = (const hb_byte_t *)_data;

    for( size_t i = 0, j = 0; i != _size;)
    {
        uint32_t octet_a = i != _size ? data[i++] : 0;
        uint32_t octet_b = i != _size ? data[i++] : 0;
        uint32_t octet_c = i != _size ? data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        if( j < totalsize ) _base64[j++] = base64_encode_table[(triple >> 3 * 6) & 0x3F];
        if( j < totalsize ) _base64[j++] = base64_encode_table[(triple >> 2 * 6) & 0x3F];
        if( j < totalsize ) _base64[j++] = base64_encode_table[(triple >> 1 * 6) & 0x3F];
        if( j < totalsize ) _base64[j++] = base64_encode_table[(triple >> 0 * 6) & 0x3F];
    }

    if( _outsize != HB_NULLPTR )
    {
        *_outsize = totalsize;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_base64_decode( const char * _base64, size_t _size, void * _data, size_t _capacity, size_t * _outsize )
{
    size_t totalsize = hb_base64_decode_size( _size );

    if( totalsize > _capacity )
    {
        return HB_FAILURE;
    }

    hb_byte_t * data = (hb_byte_t *)_data;

    for( uint32_t i = 0, j = 0; i != _size;)
    {
        uint32_t sextet_a = i != _size ? base64_decode_table[_base64[i++]] : 0;
        uint32_t sextet_b = i != _size ? base64_decode_table[_base64[i++]] : 0;
        uint32_t sextet_c = i != _size ? base64_decode_table[_base64[i++]] : 0;
        uint32_t sextet_d = i != _size ? base64_decode_table[_base64[i++]] : 0;

        uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

        if( j != totalsize ) data[j++] = (triple >> 2 * 8) & 0xFF;
        if( j != totalsize ) data[j++] = (triple >> 1 * 8) & 0xFF;
        if( j != totalsize ) data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    if( _outsize != HB_NULLPTR )
    {
        *_outsize = totalsize;
    }

    return HB_SUCCESSFUL;
}