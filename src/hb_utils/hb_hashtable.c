#include "hb_hashtable.h"
#include "hb_murmurhash64.h"

#include "hb_memory/hb_memory.h"

#include <memory.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
static void * HB_HASHTABLE_INVALID_PTR = (void *)(~0U);
//////////////////////////////////////////////////////////////////////////
typedef struct hb_hashtable_record_t
{
    uint64_t hash;

    uint8_t key[32];
    size_t key_size;

    void * element;
} hb_hashtable_record_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_hashtable_t
{
    size_t size;
    size_t capacity;

    hb_hashtable_record_t * records;
} hb_hashtable_t;
//////////////////////////////////////////////////////////////////////////
static hb_hashtable_record_t * __hb_hashtable_newbuffer( size_t _capacity )
{
    hb_hashtable_record_t * new_records = HB_NEWN( hb_hashtable_record_t, _capacity );

    for( size_t index = 0; index != _capacity; ++index )
    {
        hb_hashtable_record_t * record = new_records + index;

        record->hash = 0ULL;
        record->key[0] = 0;
        record->key_size = 0ULL;
        record->element = HB_NULLPTR;
    }

    return new_records;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_hashtable_create( size_t _capacity, hb_hashtable_t ** _ht )
{
    hb_hashtable_t * ht = HB_NEW( hb_hashtable_t );

    if( _capacity > 8 )
    {
        --_capacity;
        _capacity |= _capacity >> 16;
        _capacity |= _capacity >> 8;
        _capacity |= _capacity >> 4;
        _capacity |= _capacity >> 2;
        _capacity |= _capacity >> 1;
        ++_capacity;
    }
    else
    {
        _capacity = 8;
    }

    hb_hashtable_record_t * new_records = __hb_hashtable_newbuffer( _capacity );

    ht->size = 0;
    ht->capacity = _capacity;
    ht->records = new_records;

    *_ht = ht;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_hashtable_destroy( hb_hashtable_t * _ht )
{
    HB_DELETEN( _ht->records );
    HB_DELETE( _ht );
}
//////////////////////////////////////////////////////////////////////////
static void __hb_hashtable_push( hb_hashtable_record_t * _records, size_t _capacity, uint64_t _hash, const void * _key, size_t _size, void * _element )
{
    size_t hash_mask = _capacity - 1;
    size_t mask = (size_t)_hash;

    for( uint64_t probe = _hash; ; probe >>= 5 )
    {
        size_t index = mask & hash_mask;

        hb_hashtable_record_t * record = _records + index;

        if( record->element == HB_NULLPTR || record->element == HB_HASHTABLE_INVALID_PTR )
        {
            record->hash = _hash;
            memcpy( record->key, _key, _size );
            record->key_size = _size;
            record->element = _element;

            break;
        }

        mask = (mask << 2) + mask + (size_t)probe + 1;
    }
}
//////////////////////////////////////////////////////////////////////////
static void __hb_hashtable_rebalance( hb_hashtable_t * _ht, size_t _capacity )
{
    size_t old_capacity = _ht->capacity;
    hb_hashtable_record_t * old_records = _ht->records;

    hb_hashtable_record_t * new_records = __hb_hashtable_newbuffer( _capacity );

    for( size_t index = 0; index != old_capacity; ++index )
    {
        hb_hashtable_record_t * record = old_records + index;

        if( record->element == HB_NULLPTR || record->element == HB_HASHTABLE_INVALID_PTR )
        {
            continue;
        }

        __hb_hashtable_push( new_records, _capacity, record->hash, record->key, record->key_size, record->element );
    }

    HB_DELETEN( old_records );

    _ht->capacity = _capacity;
    _ht->records = new_records;
}
//////////////////////////////////////////////////////////////////////////
static void __hb_hashtable_increase( hb_hashtable_t * _ht )
{
    size_t new_capacity = _ht->capacity << 1;

    __hb_hashtable_rebalance( _ht, new_capacity );
}
//////////////////////////////////////////////////////////////////////////
static void __hb_hashtable_checkincrease( hb_hashtable_t * _ht )
{
    size_t test_size = _ht->size * 3 + 1;
    size_t test_capacity = _ht->capacity * 2;

    if( test_size <= test_capacity )
    {
        return;
    }

    __hb_hashtable_increase( _ht );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_hashtable_emplace( hb_hashtable_t * _ht, const void * _key, size_t _size, void * _element )
{
    __hb_hashtable_checkincrease( _ht );

    if( _size == HB_UNKNOWN_STRING_SIZE )
    {
        _size = strlen( (const char *)_key );
    }

    if( _size > 32 )
    {
        return HB_FAILURE;
    }

    uint64_t hash = hb_murmurhash64( _key, _size );

    __hb_hashtable_push( _ht->records, _ht->capacity, hash, _key, _size, _element );

    ++_ht->size;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static void * __hb_hashtable_change( hb_hashtable_record_t * _records, size_t _capacity, uint64_t _hash, const void * _key, size_t _size, void * _element )
{
    size_t hash_mask = _capacity - 1;
    size_t mask = (size_t)_hash;

    for( uint64_t probe = _hash; ; probe >>= 5 )
    {
        size_t index = mask & hash_mask;

        hb_hashtable_record_t * record = _records + index;

        if( record->hash == _hash && memcmp( record->key, _key, _size ) == 0 )
        {
            if( record->element == HB_HASHTABLE_INVALID_PTR )
            {
                record->element = _element;

                return HB_NULLPTR;
            }

            if( record->element != HB_NULLPTR )
            {
                void * prev = record->element;
                record->element = _element;

                return prev;
            }
        }

        if( record->element == HB_NULLPTR )
        {
            record->hash = _hash;
            memcpy( record->key, _key, _size );
            record->key_size = _size;
            record->element = _element;

            return HB_NULLPTR;
        }

        mask = (mask << 2) + mask + (size_t)probe + 1;
    }
}
//////////////////////////////////////////////////////////////////////////
void * hb_hashtable_change( hb_hashtable_t * _ht, const void * _key, size_t _size, void * _element )
{
    if( _size == HB_UNKNOWN_STRING_SIZE )
    {
        _size = strlen( (const char *)_key );
    }

    if( _size > 32 )
    {
        return HB_NULLPTR;
    }

    __hb_hashtable_checkincrease( _ht );

    uint64_t hash = hb_murmurhash64( _key, _size );

    void * change_element = __hb_hashtable_change( _ht->records, _ht->capacity, hash, _key, _size, _element );

    if( change_element == HB_NULLPTR )
    {
        ++_ht->size;
    }

    return change_element;
}
//////////////////////////////////////////////////////////////////////////
static void * __hb_hashtable_pop( hb_hashtable_record_t * _records, size_t _capacity, uint64_t _hash, const void * _key, size_t _size )
{
    size_t hash_mask = _capacity - 1;
    size_t mask = (size_t)_hash;

    for( uint64_t probe = _hash; ; probe >>= 5 )
    {
        size_t index = mask & hash_mask;

        hb_hashtable_record_t * record = _records + index;

        if( record->element == HB_NULLPTR )
        {
            return HB_NULLPTR;
        }

        if( record->hash == _hash && memcmp( record->key, _key, _size ) == 0 && record->element != HB_HASHTABLE_INVALID_PTR )
        {
            void * pop_element = record->element;
            record->element = HB_HASHTABLE_INVALID_PTR;

            return pop_element;
        }

        mask = (mask << 2) + mask + (size_t)probe + 1;
    }
}
//////////////////////////////////////////////////////////////////////////
void * hb_hashtable_erase( hb_hashtable_t * _ht, const void * _key, size_t _size )
{
    if( _ht->size == 0 )
    {
        return HB_NULLPTR;
    }

    if( _size == HB_UNKNOWN_STRING_SIZE )
    {
        _size = strlen( (const char *)_key );
    }

    if( _size > 32 )
    {
        return HB_NULLPTR;
    }

    uint64_t hash = hb_murmurhash64( _key, _size );

    void * erase_element = __hb_hashtable_pop( _ht->records, _ht->capacity, hash, _key, _size );

    if( erase_element != HB_NULLPTR )
    {
        --_ht->size;
    }

    return erase_element;
}
//////////////////////////////////////////////////////////////////////////
static void * __hb_hashtable_find( hb_hashtable_record_t * _records, size_t _capacity, uint64_t _hash, const void * _key, size_t _size )
{
    size_t hash_mask = _capacity - 1;
    size_t mask = (size_t)_hash;

    for( uint64_t probe = _hash; ; probe >>= 5 )
    {
        size_t index = mask & hash_mask;

        hb_hashtable_record_t * record = _records + index;

        if( record->element == HB_NULLPTR )
        {
            return HB_NULLPTR;
        }

        if( record->hash == _hash && memcmp( record->key, _key, _size ) == 0 )
        {
            if( record->element == HB_HASHTABLE_INVALID_PTR )
            {
                return HB_NULLPTR;
            }

            return record->element;
        }

        mask = (mask << 2) + mask + (size_t)probe + 1;
    }
}
//////////////////////////////////////////////////////////////////////////
void * hb_hashtable_find( hb_hashtable_t * _ht, const void * _key, size_t _size )
{
    if( _ht->size == 0 )
    {
        return HB_NULLPTR;
    }

    if( _size == HB_UNKNOWN_STRING_SIZE )
    {
        _size = strlen( (const char *)_key );
    }

    if( _size > 32 )
    {
        return HB_NULLPTR;
    }

    uint64_t hash = hb_murmurhash64( _key, _size );

    void * find_element = __hb_hashtable_find( _ht->records, _ht->capacity, hash, _key, _size );

    return find_element;
}
//////////////////////////////////////////////////////////////////////////
void hb_hashtable_reserve( hb_hashtable_t * _ht, size_t _capacity )
{
    if( _ht->capacity > _capacity )
    {
        return;
    }

    --_capacity;
    _capacity |= _capacity >> 16;
    _capacity |= _capacity >> 8;
    _capacity |= _capacity >> 4;
    _capacity |= _capacity >> 2;
    _capacity |= _capacity >> 1;
    ++_capacity;

    __hb_hashtable_rebalance( _ht, _capacity );
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_hashtable_empty( hb_hashtable_t * _ht )
{
    if( _ht->size == 0 )
    {
        return HB_FALSE;
    }

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////
size_t hb_hashtable_size( hb_hashtable_t * _ht )
{
    return _ht->size;
}
//////////////////////////////////////////////////////////////////////////
void hb_hashtable_clear( hb_hashtable_t * _ht )
{
    size_t capacity = _ht->capacity;
    hb_hashtable_record_t * values = _ht->records;

    for( size_t index = 0; index != capacity; ++index )
    {
        hb_hashtable_record_t * record = values + index;

        if( record->element == HB_NULLPTR || record->element == HB_HASHTABLE_INVALID_PTR )
        {
            continue;
        }

        record->element = HB_NULLPTR;
        record->key[0] = 0;
        record->key_size = 0;
    }

    _ht->size = 0;
}