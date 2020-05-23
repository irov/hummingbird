#include "hb_list.h"

#include "hb_memory/hb_memory.h"

//////////////////////////////////////////////////////////////////////////
typedef struct hb_list_t
{
    hb_list_element_t * prev;
    hb_list_element_t * next;
}hb_list_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_list_create( hb_list_t ** _list )
{
    hb_list_t * list = HB_NEW( hb_list_t );

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
    if( _list->next == HB_NULLPTR )
    {
        _list->next = _element;
        _list->prev = _element;
    }
    else
    {
        _element->prev = _list->prev;
        _element->next = HB_NULLPTR;
        _list->prev->next = _element;
        _list->prev = _element;
    }
}
//////////////////////////////////////////////////////////////////////////
void hb_list_push_front( hb_list_t * _list, hb_list_element_t * _element )
{
    if( _list->next == HB_NULLPTR )
    {
        _list->next = _element;
        _list->prev = _element;
    }
    else
    {
        _element->prev = HB_NULLPTR;
        _element->next = _list->next;
        _list->next->prev = _element;
        _list->next = _element;
    }
}
//////////////////////////////////////////////////////////////////////////
void hb_list_pop_back( hb_list_t * _list, hb_list_element_t ** _element )
{
    HB_UNUSED( _list );
    HB_UNUSED( _element );
}
//////////////////////////////////////////////////////////////////////////
void hb_list_pop_front( hb_list_t * _list, hb_list_element_t ** _element )
{
    HB_UNUSED( _list );
    HB_UNUSED( _element );
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_list_empty( const hb_list_t * _list )
{
    if( _list->prev == HB_NULLPTR && _list->next == HB_NULLPTR )
    {
        return HB_TRUE;
    }

    return HB_FALSE;
}