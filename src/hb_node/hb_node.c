#include "hb_node.h"

#include "hb_utils/hb_getopt.h"

//////////////////////////////////////////////////////////////////////////
static void __hb_node_create_header_in( void * _in, uint32_t _magic, uint32_t _version )
{
    hb_node_header_t * header = (hb_node_header_t *)_in;

    header->magic_number = _magic;
    header->version_number = _version;
}
//////////////////////////////////////////////////////////////////////////
static void __hb_node_create_header_out( void * _in, uint32_t _magic, uint32_t _version )
{
    hb_node_header_t * header = (hb_node_header_t *)_in;

    header->magic_number = ~_magic;
    header->version_number = _version;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_node_test_header_in( const void * _in, uint32_t _magic, uint32_t _version )
{
    const hb_node_header_t * header = (const hb_node_header_t *)_in;

    if( header->magic_number != _magic )
    {
        return HB_FAILURE;
    }

    if( header->version_number != _version )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_node_test_header_out( const void * _in, uint32_t _magic, uint32_t _version )
{
    const hb_node_header_t * header = (const hb_node_header_t *)_in;

    if( header->magic_number != ~_magic )
    {
        return HB_FAILURE;
    }

    if( header->version_number != _version )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_node_open_sharedmemory( int _argc, char * _argv[], hb_sharedmemory_handle_t * _sharedmemory )
{
    const char * sm_name;
    if( hb_getopt( _argc, _argv, "--sm", &sm_name ) == 0 )
    {
        return HB_FAILURE;
    }

    if( hb_sharedmemory_open( sm_name, HB_SHAREDMEMORY_SIZE, _sharedmemory ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_node_read_in_data( hb_sharedmemory_handle_t * _sharedmemory, void * _in, size_t _size, uint32_t _magic, uint32_t _version )
{
    if( hb_sharedmemory_read( _sharedmemory, _in, _size, HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( __hb_node_test_header_in( _in, _magic, _version ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}