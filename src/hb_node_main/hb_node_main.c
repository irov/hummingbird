#include "hb_node_main.h"

#include "hb_log/hb_log.h"
#include "hb_log_file/hb_log_file.h"
#include "hb_log_tcp/hb_log_tcp.h"
#include "hb_cache/hb_cache.h"
#include "hb_db/hb_db.h"
#include "hb_storage/hb_storage.h"

#include "hb_utils/hb_oid.h"

#include <stdlib.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
static void __hb_log_observer( const char * _category, hb_log_level_e _level, const char * _message )
{
    const char * ls = hb_log_level_string[_level];

    printf( "[%s] %s: %s\n", _category, ls, _message );
}
//////////////////////////////////////////////////////////////////////////
int main( int _argc, char * _argv[] )
{
    if( hb_log_initialize() == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_observer ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_sharedmemory_handle_t sharedmemory_handle;
    if( hb_node_open_sharedmemory( _argc, _argv, &sharedmemory_handle ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_node_config_t config;

    uint8_t in_data[20480];
    if( hb_node_read_in_data( &sharedmemory_handle, &config, &in_data, sizeof( in_data ) ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( hb_log_file_initialize( config.log_file ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }
    
    if( hb_log_tcp_initialize( config.log_uri, config.log_port ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    uint32_t components = hb_node_components_enumerator;

    if( components & e_hb_component_cache )
    {
        if( hb_cache_initialize( config.cache_uri, config.cache_port, 5 ) == HB_FAILURE )
        {
            hb_log_message( "node", HB_LOG_ERROR, "node '%s' invalid initialize [cache] component [uri %s:%u]"
                , config.name
                , config.cache_uri
                , config.cache_port
            );

            hb_node_write_error_data( &sharedmemory_handle, e_node_component_invalid_initialize );

            return EXIT_SUCCESS;
        }
    }
    
    if( components & e_hb_component_db )
    {
        if( hb_db_initialze( config.name, config.db_uri, config.db_port ) == HB_FAILURE )
        {
            hb_log_message( "node", HB_LOG_ERROR, "node '%s' invalid initialize [db] component [uri %s:%u]"
                , config.name
                , config.db_uri
                , config.db_port
            );

            hb_node_write_error_data( &sharedmemory_handle, e_node_component_invalid_initialize );

            return EXIT_SUCCESS;
        }
    }

    if( components & e_hb_component_storage )
    {
        if( hb_storage_initialize() == HB_FAILURE )
        {
            hb_log_message( "node", HB_LOG_ERROR, "node '%s' invalid initialize [storage] component"
                , config.name
            );

            hb_node_write_error_data( &sharedmemory_handle, e_node_component_invalid_initialize );

            return EXIT_SUCCESS;
        }
    }

    uint8_t out_data[1024];
    size_t out_data_size;

    hb_node_process( in_data, out_data, &out_data_size );

    hb_node_write_out_data( &sharedmemory_handle, out_data, out_data_size );
    hb_sharedmemory_destroy( &sharedmemory_handle );

    if( components & e_hb_component_storage )
    {
        hb_storage_finalize();
    }

    if( components & e_hb_component_cache )
    {
        hb_cache_finalize();
    }

    if( components & e_hb_component_db )
    {
        hb_db_finalize();
    }

    hb_log_tcp_finalize();

    hb_log_finalize();

    return EXIT_SUCCESS;
}