#include "hb_sha1.h"

#include <memory.h>

//////////////////////////////////////////////////////////////////////////
void hb_sha1_copy( hb_sha1_t * _dst, const hb_sha1_t * _src )
{
    memcpy( _dst->value, _src->value, sizeof( hb_sha1_t ) );
}
//////////////////////////////////////////////////////////////////////////
#define HB_SHA1_ROLL( value, steps ) ((value << steps) | (value >> (32 - steps)))
//////////////////////////////////////////////////////////////////////////
#define HB_SHA1_ABCDE(func, value) \
	{const uint32_t t = HB_SHA1_ROLL(a, 5) + (func) + value + e + w[round]; \
	e = d; \
	d = c; \
	c = HB_SHA1_ROLL(b, 30); \
	b = a; \
	a = t;}
//////////////////////////////////////////////////////////////////////////
static inline void __hb_sha1_clear( uint32_t * _buffer )
{
    for( uint32_t
        * it = _buffer,
        *it_end = _buffer + 17;
        it != it_end;
        ++it )
    {
        *it = 0;
    }
}
//////////////////////////////////////////////////////////////////////////
static void __hb_sha1_process( uint32_t * result, uint32_t * w )
{
    uint32_t a = result[0];
    uint32_t b = result[1];
    uint32_t c = result[2];
    uint32_t d = result[3];
    uint32_t e = result[4];

    int32_t round = 0;

    while( round != 16 )
    {
        HB_SHA1_ABCDE( (b & c) | (~b & d), 0x5a827999 );
        ++round;
    }

    while( round != 20 )
    {
        w[round] = HB_SHA1_ROLL( (w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1 );
        HB_SHA1_ABCDE( (b & c) | (~b & d), 0x5a827999 );
        ++round;
    }

    while( round != 40 )
    {
        w[round] = HB_SHA1_ROLL( (w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1 );
        HB_SHA1_ABCDE( b ^ c ^ d, 0x6ed9eba1 );
        ++round;
    }

    while( round != 60 )
    {
        w[round] = HB_SHA1_ROLL( (w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1 );
        HB_SHA1_ABCDE( (b & c) | (b & d) | (c & d), 0x8f1bbcdc );
        ++round;
    }

    while( round != 80 )
    {
        w[round] = HB_SHA1_ROLL( (w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1 );
        HB_SHA1_ABCDE( b ^ c ^ d, 0xca62c1d6 );
        ++round;
    }

    result[0] += a;
    result[1] += b;
    result[2] += c;
    result[3] += d;
    result[4] += e;
}
//////////////////////////////////////////////////////////////////////////
void hb_sha1( const void * _buffer, const hb_size_t _size, hb_sha1_t * _sha1 )
{
    uint32_t result[5] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};

    const hb_byte_t * sarray = (const hb_byte_t *)_buffer;

    uint32_t w[80];

    int32_t endOfFullBlocks = _size - 64;
    int32_t endCurrentBlock;
    int32_t currentBlock = 0;

    while( currentBlock <= endOfFullBlocks )
    {
        endCurrentBlock = currentBlock + 64;

        for( int32_t roundPos = 0; currentBlock < endCurrentBlock; currentBlock += 4 )
        {
            uint32_t w0 = ((uint32_t)sarray[currentBlock + 0]) << 24;
            uint32_t w1 = ((uint32_t)sarray[currentBlock + 1]) << 16;
            uint32_t w2 = ((uint32_t)sarray[currentBlock + 2]) << 8;
            uint32_t w3 = ((uint32_t)sarray[currentBlock + 3]) << 0;

            w[roundPos++] = w0 | w1 | w2 | w3;
        }

        __hb_sha1_process( result, w );
    }

    endCurrentBlock = _size - currentBlock;

    __hb_sha1_clear( w );

    int32_t lastBlockBytes = 0;

    for( ; lastBlockBytes != endCurrentBlock; ++lastBlockBytes )
    {
        w[lastBlockBytes >> 2] |= (uint32_t)sarray[lastBlockBytes + currentBlock] << ((3 - (lastBlockBytes & 3)) << 3);
    }

    w[lastBlockBytes >> 2] |= 0x80 << ((3 - (lastBlockBytes & 3)) << 3);

    if( endCurrentBlock >= 56 )
    {
        __hb_sha1_process( result, w );
        __hb_sha1_clear( w );
    }

    w[15] = _size << 3;
    __hb_sha1_process( result, w );

    for( int32_t hashByte = 20; --hashByte >= 0; )
    {
        _sha1->value[hashByte] = (result[hashByte >> 2] >> (((3 - hashByte) & 0x3) << 3)) & 0xff;
    }
}