#ifndef HB_NODE_H_
#define HB_NODE_H_

#include "hb_config/hb_config.h"
#include "hb_sharedmemory/hb_sharedmemory.h"

typedef struct hb_node_header_t
{
    hb_magic_t magic_number;
    hb_version_t version_number;
} hb_node_header_t;

hb_result_t hb_node_open_sharedmemory( int _argc, char * _argv[], hb_sharedmemory_handle_t * _sharedmemory );
hb_result_t hb_node_write_in_data( hb_sharedmemory_handle_t * _sharedmemory, const void * _data, size_t _size, hb_magic_t _magic, hb_version_t _version );
hb_result_t hb_node_read_in_data( hb_sharedmemory_handle_t * _sharedmemory, void * _data, size_t _size, hb_magic_t _magic, hb_version_t _version );
hb_result_t hb_node_write_out_data( hb_sharedmemory_handle_t * _sharedmemory, const void * _data, size_t _size, hb_magic_t _magic, hb_version_t _version );
hb_result_t hb_node_read_out_data( hb_sharedmemory_handle_t * _sharedmemory, void * _data, size_t _size, hb_magic_t _magic, hb_version_t _version );

#endif
