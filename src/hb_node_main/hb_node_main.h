#ifndef HB_NODE_MAIN_H_
#define HB_NODE_MAIN_H_

#include "hb_node/hb_node.h"

typedef enum hb_component_type_e
{
    e_hb_component_cache = 1 << 0,
    e_hb_component_db = 1 << 1,
    e_hb_component_storage = 1 << 2,
} hb_component_type_e;

extern uint32_t hb_node_components_enumerator;

extern hb_result_t hb_node_process( const void * _data, void * _out, size_t * _size );

#endif
