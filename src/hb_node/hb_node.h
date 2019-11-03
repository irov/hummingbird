#ifndef HB_NODE_H_
#define HB_NODE_H_

#include "hb_config/hb_config.h"
#include "hb_sharedmemory/hb_sharedmemory.h"

typedef enum hb_node_code_t
{
    e_node_ok,
    e_node_component_invalid_initialize,
} hb_node_code_t;

typedef struct hb_node_config_t
{
    char name[32];

    char cache_uri[128];
    uint16_t cache_port;

    char db_uri[128];
    uint16_t db_port;

    char log_uri[128];
    uint16_t log_port;
} hb_node_config_t;

hb_result_t hb_node_open_sharedmemory( int _argc, char * _argv[], hb_sharedmemory_handle_t * _sharedmemory );
hb_result_t hb_node_write_in_data( hb_sharedmemory_handle_t * _sharedmemory, const void * _data, size_t _size, hb_node_config_t * _config );
hb_result_t hb_node_read_in_data( hb_sharedmemory_handle_t * _sharedmemory, hb_node_config_t * _config, void * _data, size_t _capacity );
hb_result_t hb_node_write_out_data( hb_sharedmemory_handle_t * _sharedmemory, const void * _data, size_t _size );
hb_result_t hb_node_write_error_data( hb_sharedmemory_handle_t * _sharedmemory, hb_node_code_t _code );
hb_result_t hb_node_read_out_data( hb_sharedmemory_handle_t * _sharedmemory, void * _data, size_t _size, hb_node_code_t * _code );

#endif
