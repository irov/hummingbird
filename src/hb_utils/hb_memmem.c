#include "hb_memmem.h"

#include <memory.h>

//////////////////////////////////////////////////////////////////////////
hb_size_t hb_memsize( const void * _begin, const void * _end )
{
    return (const hb_byte_t *)_end - (const hb_byte_t *)_begin;
}
//////////////////////////////////////////////////////////////////////////
const void * hb_memadvance( const void * _buffer, hb_size_t _offset )
{
    return (const void *)((const hb_byte_t *)_buffer + _offset);
}
//////////////////////////////////////////////////////////////////////////
const void * hb_memmem( const void * _buffer, hb_size_t _size, const void * _subbuffer, hb_size_t _subsize, hb_size_t * _offset )
{
    if( _size == 0 || _subsize == 0 )
    {
        return HB_NULLPTR;
    }

    const hb_byte_t * carriage = (const hb_byte_t *)_buffer;

    hb_size_t step = 0;

    while( _size > step + _subsize )
    {
        int32_t r = memcmp( carriage, _subbuffer, _subsize );

        if( r == 0 )
        {
            *_offset = step;

            return carriage;
        }

        ++carriage;
        ++step;
    }

    return HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
const void * hb_memmeme( const void * _buffer, hb_size_t _size, const void * _subbuffer, hb_size_t _subsize, hb_size_t * _offset )
{
    if( _size == 0 || _subsize == 0 )
    {
        return HB_NULLPTR;
    }

    const hb_byte_t * carriage = (const hb_byte_t *)_buffer;

    hb_size_t step = 0;

    while( _size > step + _subsize )
    {
        int32_t r = memcmp( carriage, _subbuffer, _subsize );

        if( r == 0 )
        {
            *_offset = step + _subsize;

            return carriage + _subsize;
        }

        ++carriage;
        ++step;
    }

    return HB_NULLPTR;
}