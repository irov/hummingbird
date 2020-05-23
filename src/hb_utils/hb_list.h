#ifndef HB_LIST_H_
#define HB_LIST_H_

#include "hb_config/hb_config.h"

typedef struct hb_list_t hb_list_t;

hb_result_t hb_list_create( hb_list_t ** _list );
void hb_list_destroy( hb_list_t * _list );

typedef struct hb_list_element_t
{
    struct hb_list_element_t * prev;
    struct hb_list_element_t * next;
}hb_list_element_t;

void hb_list_push_back( hb_list_t * _list, hb_list_element_t * _element );
void hb_list_push_front( hb_list_t * _list, hb_list_element_t * _element );
void hb_list_pop_back( hb_list_t * _list, hb_list_element_t ** _element );
void hb_list_pop_front( hb_list_t * _list, hb_list_element_t ** _element );
hb_bool_t hb_list_empty( const hb_list_t * _list );

#endif
