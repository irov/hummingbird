#include "hb_node_main.h"

#include "hb_log/hb_log.h"
#include "hb_log_file/hb_log_file.h"
#include "hb_log_tcp/hb_log_tcp.h"
#include "hb_cache/hb_cache.h"
#include "hb_db/hb_db.h"
#include "hb_storage/hb_storage.h"

#include "hb_utils/hb_oid.h"
#include "hb_utils/hb_date.h"

#include <stdlib.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
static void __hb_log_observer( const char * _category, hb_log_level_t _level, const char * _file, uint32_t _line, const char * _message, void * _ud )
{
    HB_UNUSED( _ud );

    const char * ls = hb_log_level_string[_level];

    printf( "%s [%s:%u] %s: %s\n", ls, _file, _line, _category, _message );
}
//////////////////////////////////////////////////////////////////////////
int main( int _argc, char * _argv[] )
{
    if( hb_log_initialize() == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_observer, HB_NULLPTR ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_sharedmemory_handle_t * sharedmemory_handle;
    if( hb_node_open_sharedmemory( _argc, _argv, &sharedmemory_handle ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_node_config_t config;

    uint8_t in_data[20480];
    if( hb_node_read_in_data( sharedmemory_handle, &config, in_data, sizeof( in_data ) ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_date_t date;
    hb_date( &date );

#ifdef HB_DEBUG
    uint32_t sharedmemory_id = hb_sharedmemory_get_id( sharedmemory_handle );

    char log_file[HB_MAX_PATH];
    sprintf( log_file, "%s[%u]_log_%u_%u_%u_%u_%u_%u.log"
        , config.name
        , sharedmemory_id
        , date.year
        , date.mon
        , date.mday
        , date.hour
        , date.min
        , date.sec
    );

    if( hb_log_file_initialize( log_file ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }
#endif
    
    if( hb_log_tcp_initialize( config.log_uri, config.log_port ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    uint32_t components = hb_node_components_enumerator;

    if( components & e_hb_component_cache )
    {
        if( hb_cache_initialize( config.cache_uri, config.cache_port, config.cache_timeout ) == HB_FAILURE )
        {
            HB_LOG_MESSAGE_ERROR( "node", "node '%s' invalid initialize [cache] component [uri %s:%u]"
                , config.name
                , config.cache_uri
                , config.cache_port
            );

            hb_node_write_error_data( sharedmemory_handle, e_node_component_invalid_initialize, "invalid initialize cache" );

            return EXIT_SUCCESS;
        }
    }
    
    if( components & e_hb_component_db )
    {
        if( hb_db_initialze( config.name, config.db_uri, config.db_port ) == HB_FAILURE )
        {
            HB_LOG_MESSAGE_ERROR( "node", "node '%s' invalid initialize [db] component [uri %s:%u]"
                , config.name
                , config.db_uri
                , config.db_port
            );

            hb_node_write_error_data( sharedmemory_handle, e_node_component_invalid_initialize, "invalid initialize db" );

            return EXIT_SUCCESS;
        }
    }

    if( components & e_hb_component_storage )
    {
        if( hb_storage_initialize() == HB_FAILURE )
        {
            HB_LOG_MESSAGE_ERROR( "node", "node '%s' invalid initialize [storage] component"
                , config.name
            );

            hb_node_write_error_data( sharedmemory_handle, e_node_component_invalid_initialize, "invalid initialize storage" );

            return EXIT_SUCCESS;
        }
    }

    uint8_t out_data[1024];
    size_t out_data_size;

    hb_node_process( in_data, out_data, &out_data_size );

    hb_node_write_out_data( sharedmemory_handle, out_data, out_data_size );
    hb_sharedmemory_destroy( sharedmemory_handle );

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

    hb_log_remove_observer( &__hb_log_observer, HB_NULLPTR );

    hb_log_finalize();

    return EXIT_SUCCESS;
}