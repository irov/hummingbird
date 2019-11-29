#include "hb_node.h"

#include "hb_db/hb_db.h"
#include "hb_log/hb_log.h"
#include "hb_storage/hb_storage.h"
#include "hb_script/hb_script.h"

#include "hb_utils/hb_getopt.h"

#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>

typedef uint32_t hb_magic_t;
typedef uint32_t hb_version_t;

#ifndef HB_MAGIC_NUMBER
#define HB_MAGIC_NUMBER(A,B,C,D) ((A << 0) + (B << 8) + (C << 16) + (D << 24))
#endif
//////////////////////////////////////////////////////////////////////////
static const uint32_t hb_node_magic_number = HB_MAGIC_NUMBER( 'N', 'O', 'D', 'E' );
static const uint32_t hb_node_version_number = 1;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_node_header_t
{
    hb_magic_t magic_number;
    hb_version_t version_number;
} hb_node_header_t;
//////////////////////////////////////////////////////////////////////////
static void __hb_node_create_header_in( hb_node_header_t * _header )
{
    _header->magic_number = hb_node_magic_number;
    _header->version_number = hb_node_version_number;
}
//////////////////////////////////////////////////////////////////////////
static void __hb_node_create_header_out( hb_node_header_t * _header )
{
    _header->magic_number = ~hb_node_magic_number;
    _header->version_number = hb_node_version_number;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_node_test_header_in( const hb_node_header_t * _header )
{
    if( _header->magic_number != hb_node_magic_number )
    {
        return HB_FAILURE;
    }

    if( _header->version_number != hb_node_version_number )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_node_test_header_out( const hb_node_header_t * _header )
{
    if( _header->magic_number != ~hb_node_magic_number )
    {
        return HB_FAILURE;
    }

    if( _header->version_number != hb_node_version_number )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_node_open_sharedmemory( int _argc, char * _argv[], hb_sharedmemory_handle_t ** _sharedmemory )
{
    const char * sm_id;
    if( hb_getopt( _argc, _argv, "--sm", &sm_id ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    uint32_t id;
    if( sscanf( sm_id, "%u", &id ) != 1 )
    {
        return HB_FAILURE;
    }

    if( hb_sharedmemory_open( id, HB_SHAREDMEMORY_SIZE, _sharedmemory ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_node_write_in_data( hb_sharedmemory_handle_t * _sharedmemory, const void * _data, size_t _size, const hb_node_config_t * _config )
{
    hb_sharedmemory_rewind( _sharedmemory );

    hb_node_header_t header;
    __hb_node_create_header_in( &header );

    if( hb_sharedmemory_write( _sharedmemory, &header, sizeof( header ) ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_sharedmemory_write( _sharedmemory, _config, sizeof( hb_node_config_t ) ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_sharedmemory_write( _sharedmemory, _data, _size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_node_read_in_data( hb_sharedmemory_handle_t * _sharedmemory, hb_node_config_t * _config, void * _data, size_t _capacity )
{
    hb_sharedmemory_rewind( _sharedmemory );

    hb_node_header_t header;
    if( hb_sharedmemory_read( _sharedmemory, &header, sizeof( header ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( __hb_node_test_header_in( &header ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_sharedmemory_read( _sharedmemory, _config, sizeof( hb_node_config_t ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_sharedmemory_read( _sharedmemory, _data, _capacity, HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_node_write_out_data( hb_sharedmemory_handle_t * _sharedmemory, const void * _data, size_t _size )
{
    hb_sharedmemory_rewind( _sharedmemory );

    hb_node_header_t header;
    __hb_node_create_header_out( &header );

    if( hb_sharedmemory_write( _sharedmemory, &header, sizeof( header ) ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    uint32_t code = e_node_ok;

    if( hb_sharedmemory_write( _sharedmemory, &code, sizeof( code ) ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_sharedmemory_write( _sharedmemory, _data, _size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_node_write_error_data( hb_sharedmemory_handle_t * _sharedmemory, hb_node_code_t _code, const char * _format, ... )
{
    if( _code == e_node_ok )
    {
        return HB_FAILURE;
    }

    hb_sharedmemory_rewind( _sharedmemory );

    hb_node_header_t header;
    __hb_node_create_header_out( &header );

    if( hb_sharedmemory_write( _sharedmemory, &header, sizeof( header ) ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    uint32_t u32_code = _code;
    if( hb_sharedmemory_write( _sharedmemory, &u32_code, sizeof( u32_code ) ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    va_list args;
    va_start( args, _format );

    char reason[1024] = {'\0'};
    int n = vsprintf( reason, _format, args );

    va_end( args );

    if( n >= 1024 )
    {
        return HB_FAILURE;
    }

    if( hb_sharedmemory_write( _sharedmemory, reason, n + 1 ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_node_read_out_data( hb_sharedmemory_handle_t * _sharedmemory, void * _data, size_t _size, hb_node_code_t * _code, char * _reason )
{
    hb_sharedmemory_rewind( _sharedmemory );

    hb_node_header_t header;
    if( hb_sharedmemory_read( _sharedmemory, &header, sizeof( header ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( __hb_node_test_header_out( &header ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    uint32_t u32_code;
    if( hb_sharedmemory_read( _sharedmemory, &u32_code, sizeof( u32_code ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    *_code = u32_code;

    if( *_code != 0 )
    {
        if( hb_sharedmemory_read( _sharedmemory, _reason, 1024, HB_NULLPTR ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        return HB_SUCCESSFUL;
    }

    if( hb_sharedmemory_read( _sharedmemory, _data, _size, HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////