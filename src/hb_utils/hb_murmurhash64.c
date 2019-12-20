#include "hb_murmurhash64.h"

/*
 * Wiki: https://en.wikipedia.org/wiki/MurmurHash
 */
uint64_t hb_murmurhash64( const void * _data, size_t _size )
{
	const uint64_t m = 0xc6a4a7935bd1e995;
	const uint64_t n = 0x248bab1ccf968043;
	const uint32_t r = 47;
	
    uint64_t h = (m * _size) ^ n;

	const uint64_t * data64 = (const uint64_t *)_data;
    const uint64_t * end64 = data64 + (_size >> 4);

	while( data64 != end64 )
	{
        uint64_t k = *data64++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const uint8_t * tail8 = (const uint8_t *)data64;

    switch( _size & 7 )
    {
    case 7: h ^= ((uint64_t)tail8[6]) << 48;
	case 6: h ^= ((uint64_t)tail8[5]) << 40;
    case 5: h ^= ((uint64_t)tail8[4]) << 32;
	case 4: h ^= ((uint64_t)tail8[3]) << 24;
    case 3: h ^= ((uint64_t)tail8[2]) << 16;
	case 2: h ^= ((uint64_t)tail8[1]) << 8;
	case 1: h ^= ((uint64_t)tail8[0]) << 0;
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}
