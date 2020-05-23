#include "hb_list.h"

#include "hb_memory/hb_memory.h"

//////////////////////////////////////////////////////////////////////////
typedef struct hb_list_t
{
    uint32_t count;

    hb_list_element_t * prev;
    hb_list_element_t * next;
}hb_list_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_list_create( hb_list_t ** _list )
{
    hb_list_t * list = HB_NEW( hb_list_t );

    list->count = 0;
    list->prev = HB_NULLPTR;
    list->next = HB_NULLPTR;

    *_list = list;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_list_destroy( hb_list_t * _list )
{
    HB_DELETE( _list );
}
//////////////////////////////////////////////////////////////////////////
void hb_list_push_back( hb_list_t * _list, hb_list_element_t * _element )
{
    ++_list->count;

    if( _list->next == HB_NULLPTR )
    {
        _element->prev = HB_NULLPTR;
        _element->next = HB_NULLPTR;
        _list->next = _element;
        _list->prev = _element;

        return;
    }

    _element->prev = _list->prev;
    _element->next = HB_NULLPTR;
    _list->prev->next = _element;
    _list->prev = _element;
}
//////////////////////////////////////////////////////////////////////////
void hb_list_push_front( hb_list_t * _list, hb_list_element_t * _element )
{
    ++_list->count;

    if( _list->next == HB_NULLPTR )
    {
        _element->prev = HB_NULLPTR;
        _element->next = HB_NULLPTR;
        _list->next = _element;
        _list->prev = _element;

        return;
    }

    _element->prev = HB_NULLPTR;
    _element->next = _list->next;
    _list->next->prev = _element;
    _list->next = _element;
}
//////////////////////////////////////////////////////////////////////////
void hb_list_pop_back( hb_list_t * _list, hb_list_element_t ** _element )
{
    if( _list->next == HB_NULLPTR )
    {
        *_element = HB_NULLPTR;

        return;
    }

    --_list->count;

    hb_list_element_t * element = _list->prev;

    if( _element != HB_NULLPTR )
    {
        *_element = element;
    }

    if( _list->prev == _list->next )
    {
        _list->prev = HB_NULLPTR;
        _list->next = HB_NULLPTR;
    }
    else
    {
        element->prev->next = HB_NULLPTR;
        _list->prev = element->prev;
    }

    element->next = HB_NULLPTR;
    element->prev = HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
void hb_list_pop_front( hb_list_t * _list, hb_list_element_t ** _element )
{
    if( _list->next == HB_NULLPTR )
    {
        *_element = HB_NULLPTR;

        return;
    }

    --_list->count;

    hb_list_element_t * element = _list->next;

    if( _element != HB_NULLPTR )
    {
        *_element = element;
    }

    if( _list->prev == _list->next )
    {
        _list->prev = HB_NULLPTR;
        _list->next = HB_NULLPTR;
    }
    else
    {
        element->next->prev = HB_NULLPTR;
        _list->next = element->next;
    }

    element->next = HB_NULLPTR;
    element->prev = HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
void hb_list_pop_element( hb_list_t * _list, hb_list_element_t * _element )
{
    --_list->count;

    if( _element->prev == HB_NULLPTR && _element->next == HB_NULLPTR )
    {
        _list->next = HB_NULLPTR;
        _list->prev = HB_NULLPTR;

        return;
    }

    if( _element->prev == HB_NULLPTR )
    {
        _list->next = _element->next;
        _element->next->prev = HB_NULLPTR;
    }
    else if( _element->next == HB_NULLPTR )
    {
        _list->prev = _element->prev;
        _element->prev->next = HB_NULLPTR;
    }
    else
    {
        _element->prev->next = _element->next;
        _element->next->prev = _element->prev;
    }

    _element->next = HB_NULLPTR;
    _element->prev = HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
uint32_t hb_list_count( const hb_list_t * _list )
{
    uint32_t count = _list->count;

    return count;
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_list_empty( const hb_list_t * _list )
{
    return _list->count == 0;
}