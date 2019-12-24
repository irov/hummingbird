#include "hb_matching.h"

#include "hb_log/hb_log.h"
#include "hb_log_tcp/hb_log_tcp.h"
#include "hb_log_file/hb_log_file.h"
#include "hb_http/hb_http.h"
#include "hb_json/hb_json.h"
#include "hb_cache/hb_cache.h"
#include "hb_utils/hb_memmem.h"
#include "hb_utils/hb_multipart.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_date.h"
#include "hb_utils/hb_sleep.h"
#include "hb_utils/hb_oid.h"

#include "hb_utils/hb_hashtable.h"

#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
static void __hb_log_observer( const char * _category, hb_log_level_t _level, const char * _file, uint32_t _line, const char * _message, void * _ud )
{
    HB_UNUSED( _ud );

    const char * ls = hb_log_level_string[_level];

    printf( "%s [%s:%u] %s: %s\n", ls, _file, _line, _category, _message );
}
////////////////////////////////////////////////////////////////////////
static void __hb_grid_request( struct evhttp_request * _request, void * _ud )
{
    const char * host = evhttp_request_get_host( _request );
    HB_UNUSED( host );

    const char * uri = evhttp_request_get_uri( _request );
    HB_UNUSED( uri );

    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );

    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    enum evhttp_cmd_type command_type = evhttp_request_get_command( _request );
    
    if( command_type == EVHTTP_REQ_OPTIONS )
    {
        struct evkeyvalq * output_headers = evhttp_request_get_output_headers( _request );

        evhttp_add_header( output_headers, "Access-Control-Allow-Origin", "*" );
        evhttp_add_header( output_headers, "Access-Control-Allow-Headers", "*" );
        evhttp_add_header( output_headers, "Access-Control-Allow-Methods", "POST" );
        evhttp_add_header( output_headers, "Content-Type", "application/json" );

        evhttp_send_reply( _request, HTTP_OK, "", output_buffer );

        return;
    }

    hb_matching_process_handle_t * process = (hb_matching_process_handle_t *)_ud;
    HB_UNUSED( process );

    int32_t response_code = HTTP_OK;

    size_t response_data_size = 2;
    char response_data[HB_GRID_REQUEST_DATA_MAX_SIZE];
    strcpy( response_data, "{}" );

    hb_json_handle_t * json;
    if( hb_http_get_request_json( _request, &json ) == HB_FAILURE )
    {
        return;
    }

    hb_oid16_t moid16;
    if( hb_json_get_field_oid16( json, "moid16", &moid16 ) == HB_FAILURE )
    {
        hb_json_destroy( json );

        return;
    }

    hb_oid16_t uoid16;
    if( hb_json_get_field_oid16( json, "uoid16", &uoid16 ) == HB_FAILURE )
    {
        hb_json_destroy( json );

        return;
    }

    hb_matching_room_t * room_found = (hb_matching_room_t *)hb_hashtable_find( process->ht, moid16, sizeof( hb_oid16_t ) );

    if( room_found == HB_NULLPTR )
    {
        hb_oid_t moid;
        hb_oid_base16_decode( moid16, &moid );

        const char * fields[] = {"count", "dispersion"};

        hb_db_value_handle_t values[2];
        if( hb_db_get_values( process->db_collection_matching, moid, fields, values, 2 ) == HB_FAILURE )
        {
            hb_json_destroy( json );

            return;
        }

        hb_matching_room_t * new_room = HB_NEW( hb_matching_room_t );
        
        new_room->count = values[0].u.i32;
        new_room->dispersion = values[1].u.i32;

        new_room->users = HB_NEWN( hb_matching_user_t, 64 );
        new_room->users_count = 0;
        new_room->users_capacity = 64;

        hb_hashtable_emplace( process->ht, moid16, sizeof( hb_oid16_t ), new_room );
    }

    hb_json_destroy( json );

    //char cmd[128] = { '\0' };
    //int count = sscanf( uri, "/%[^'/']", cmd );

    //if( count == 0 )
    //{
        //evhttp_send_reply( _request, HTTP_BADREQUEST, "", output_buffer );

        //return;
    //}

    //if( strcmp( cmd, "create" ) == 0 )
    //{


        //hb_json_destroy( json );
    //}
    //else if( strcmp( cmd, "join" ) == 0 )
    //{

    //}

    evbuffer_add( output_buffer, response_data, response_data_size );

    struct evkeyvalq * output_headers = evhttp_request_get_output_headers( _request );

    evhttp_add_header( output_headers, "Access-Control-Allow-Origin", "*" );
    evhttp_add_header( output_headers, "Access-Control-Allow-Headers", "*" );
    evhttp_add_header( output_headers, "Access-Control-Allow-Methods", "POST" );
    evhttp_add_header( output_headers, "Content-Type", "application/json" );

    evhttp_send_reply( _request, response_code, "", output_buffer );
}
//////////////////////////////////////////////////////////////////////////
static void __hb_ev_thread_base( void * _ud )
{
    HB_UNUSED( _ud );

    hb_matching_process_handle_t * handle = (hb_matching_process_handle_t *)_ud;

    struct event_base * base = event_base_new();
    HB_UNUSED( base );

    struct evhttp * http_server = evhttp_new( base );
    HB_UNUSED( http_server );

    evhttp_set_allowed_methods( http_server, EVHTTP_REQ_POST | EVHTTP_REQ_OPTIONS );

    evhttp_set_gencb( http_server, &__hb_grid_request, handle );

    if( *handle->ev_socket == -1 )
    {
        struct evhttp_bound_socket * bound_socket = evhttp_bind_socket_with_handle( http_server, handle->matching_uri, handle->matching_port );
        HB_UNUSED( bound_socket );

        *handle->ev_socket = evhttp_bound_socket_get_fd( bound_socket );
    }
    else
    {
        evhttp_accept_socket( http_server, *handle->ev_socket );
    }

    event_base_dispatch( base );
    
    evhttp_free( http_server );
}
//////////////////////////////////////////////////////////////////////////
int main( int _argc, char * _argv[] )
{
    HB_UNUSED( _argc );
    HB_UNUSED( _argv );

    hb_log_initialize();

    if( hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_observer, HB_NULLPTR ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_hashtable_t * ht;
    hb_hashtable_create( 32, &ht );

#ifdef WIN32
    const WORD wVersionRequested = MAKEWORD( 2, 2 );

    WSADATA wsaData;
    int err = WSAStartup( wVersionRequested, &wsaData );

    if( err != 0 )
    {
        return EXIT_FAILURE;
    }
#endif

    if( hb_log_tcp_initialize( "127.0.0.1", 5044 ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

#ifdef HB_DEBUG
    hb_date_t date;
    hb_date( &date );

    char logfile[HB_MAX_PATH];
    sprintf( logfile, "log_matching_%u_%u_%u_%u_%u_%u.log"
        , date.year
        , date.mon
        , date.mday
        , date.hour
        , date.min
        , date.sec );

    if( hb_log_file_initialize( logfile ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }
#endif

    const char * config_file = HB_NULLPTR;
    hb_getopt( _argc, _argv, "--config", &config_file );

    hb_matching_config_t * config = HB_NEW( hb_matching_config_t );

    uint32_t max_thread = 16;

    char matching_uri[128];
    strcpy( matching_uri, "127.0.0.1" );
    uint16_t matching_port = 5556;

    strcpy( config->name, "mt" );
    strcpy( config->cache_uri, "127.0.0.1" );
    config->cache_port = 6379;
    config->cache_timeout = 2000;
    strcpy( config->db_uri, "127.0.0.1" );
    config->db_port = 27017;
    strcpy( config->log_uri, "127.0.0.1" );
    config->log_port = 5044;

    if( config_file != HB_NULLPTR )
    {
        hb_json_handle_t * json_handle;
        if( hb_json_load( config_file, &json_handle ) == HB_FAILURE )
        {
            HB_LOG_MESSAGE_CRITICAL( "grid", "config file '%s' wrong json"
                , config_file
            );

            return EXIT_FAILURE;
        }

        int64_t config_max_thread = 0;
        hb_json_get_field_integer( json_handle, "max_thread", &config_max_thread, max_thread );
        max_thread = (uint32_t)config_max_thread;

        const char * config_matching_uri = HB_NULLPTR;
        hb_json_get_field_string( json_handle, "matching_uri", &config_matching_uri, HB_NULLPTR, matching_uri );
        strcpy( matching_uri, config_matching_uri );

        int64_t config_matching_port = 0;
        hb_json_get_field_integer( json_handle, "matching_port", &config_matching_port, matching_port );
        matching_port = (uint16_t)config_matching_port;

        const char * name = HB_NULLPTR;
        hb_json_get_field_string( json_handle, "name", &name, HB_NULLPTR, config->name );
        strcpy( config->name, name );

        const char * cache_uri = HB_NULLPTR;
        hb_json_get_field_string( json_handle, "cache_uri", &cache_uri, HB_NULLPTR, config->cache_uri );
        strcpy( config->cache_uri, cache_uri );

        int64_t cache_port = 0;
        hb_json_get_field_integer( json_handle, "cache_port", &cache_port, config->cache_port );
        config->cache_port = (uint16_t)cache_port;

        int64_t cache_timeout = 0;
        hb_json_get_field_integer( json_handle, "cache_timeout", &cache_timeout, config->cache_timeout );
        config->cache_timeout = (uint16_t)cache_timeout;

        const char * db_uri = HB_NULLPTR;
        hb_json_get_field_string( json_handle, "db_uri", &db_uri, HB_NULLPTR, config->db_uri );
        strcpy( config->db_uri, db_uri );

        int64_t db_port = 0;
        hb_json_get_field_integer( json_handle, "db_port", &db_port, config->db_port );
        config->db_port = (uint16_t)db_port;

        const char * log_uri = HB_NULLPTR;
        hb_json_get_field_string( json_handle, "log_uri", &log_uri, HB_NULLPTR, config->log_uri );
        strcpy( config->log_uri, log_uri );

        int64_t log_port = 0;
        hb_json_get_field_integer( json_handle, "log_port", &log_port, config->log_port );
        config->log_port = (uint16_t)log_port;        

        hb_json_destroy( json_handle );
    }

    hb_db_collection_handle_t * db_collection_matching;

    if( hb_db_get_collection( "hb", "hb_matching", &db_collection_matching ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "matching", "invalid initialize script: db not found collection '%s'"
            , "hb_matching"
        );

        return EXIT_FAILURE;
    }

    HB_LOG_MESSAGE_INFO( "grid", "start matching with config:" );
    HB_LOG_MESSAGE_INFO( "grid", "------------------------------------" );
    HB_LOG_MESSAGE_INFO( "grid", "max_thread: %u", max_thread );
    HB_LOG_MESSAGE_INFO( "grid", "matching_uri: %s", matching_uri );
    HB_LOG_MESSAGE_INFO( "grid", "matching_port: %u", matching_port );
    HB_LOG_MESSAGE_INFO( "grid", "name: %s", config->name );
    HB_LOG_MESSAGE_INFO( "grid", "cache_uri: %s", config->cache_uri );
    HB_LOG_MESSAGE_INFO( "grid", "cache_port: %u", config->cache_port );
    HB_LOG_MESSAGE_INFO( "grid", "cache_timeout: %u", config->cache_timeout );
    HB_LOG_MESSAGE_INFO( "grid", "db_uri: %s", config->db_uri );
    HB_LOG_MESSAGE_INFO( "grid", "db_port: %u", config->db_port );
    HB_LOG_MESSAGE_INFO( "grid", "log_uri: %s", config->log_uri );
    HB_LOG_MESSAGE_INFO( "grid", "log_port: %u", config->log_port );
    HB_LOG_MESSAGE_INFO( "grid", "------------------------------------" );

    hb_matching_process_handle_t * process_handles = HB_NEWN( hb_matching_process_handle_t, max_thread );

    evutil_socket_t ev_socket = -1;
    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_matching_process_handle_t * process_handle = process_handles + i;

        strcpy( process_handle->matching_uri, matching_uri );
        process_handle->matching_port = matching_port;
        process_handle->ev_socket = &ev_socket;
        process_handle->config = config;
        process_handle->db_collection_matching = db_collection_matching;
        process_handle->ht = ht;

        if( hb_thread_create( &__hb_ev_thread_base, process_handle, &process_handle->thread ) == HB_FAILURE )
        {
            continue;
        }

        hb_sleep( 100 ); //hack
    }

    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_matching_process_handle_t * process_handle = process_handles + i;

        hb_thread_join( process_handle->thread );
    }

    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_matching_process_handle_t * process_handle = process_handles + i;

        hb_thread_destroy( process_handle->thread );
    }

    hb_db_destroy_collection( db_collection_matching );

    HB_DELETEN( process_handles );

    HB_DELETE( config );

#ifdef WIN32
    WSACleanup();
#endif

    hb_log_remove_observer( &__hb_log_observer, HB_NULLPTR );

#ifdef HB_DEBUG
    hb_log_file_finalize();
#endif

    hb_log_finalize();

    return EXIT_SUCCESS;
}