#include "hb_grid.h"

#include "hb_memory/hb_memory.h"
#include "hb_log/hb_log.h"
#include "hb_log_console/hb_log_console.h"
#include "hb_log_tcp/hb_log_tcp.h"
#include "hb_log_file/hb_log_file.h"
#include "hb_http/hb_http.h"
#include "hb_db/hb_db.h"
#include "hb_storage/hb_storage.h"
#include "hb_json/hb_json.h"
#include "hb_cache/hb_cache.h"
#include "hb_utils/hb_memmem.h"
#include "hb_utils/hb_multipart.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_base64.h"
#include "hb_utils/hb_date.h"
#include "hb_utils/hb_sleep.h"

#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_grid_cmd_inittab_t
{
    const char * name;
    hb_http_code_t( *request )(hb_grid_request_handle_t * _args);
} hb_grid_cmd_inittab_t;
//////////////////////////////////////////////////////////////////////////
extern hb_http_code_t hb_grid_request_newaccount( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_loginaccount( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_newproject( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_upload( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_newuser( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_loginuser( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_api( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_avatar( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_command( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_setusernickname( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_setleaderscore( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_getleaderrank( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_getleaderboard( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_newmessageschannel( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_postmessageschannel( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_getmessageschannel( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_neweventstopic( hb_grid_request_handle_t * _args );
extern hb_http_code_t hb_grid_request_geteventstopic( hb_grid_request_handle_t * _args );
//////////////////////////////////////////////////////////////////////////
static const hb_grid_cmd_inittab_t grid_cmds[] =
{
    {"newaccount", &hb_grid_request_newaccount},
    {"loginaccount", &hb_grid_request_loginaccount},
    {"newproject", &hb_grid_request_newproject},
    {"upload", &hb_grid_request_upload},
    {"newuser", &hb_grid_request_newuser},
    {"loginuser", &hb_grid_request_loginuser},
    {"api", &hb_grid_request_api},
    {"setusernickname", &hb_grid_request_setusernickname},
    {"setleaderscore", &hb_grid_request_setleaderscore},
    {"getleaderrank", &hb_grid_request_getleaderrank},
    {"getleaderboard", &hb_grid_request_getleaderboard},
    {"newmessageschannel", &hb_grid_request_newmessageschannel},
    {"postmessageschannel", &hb_grid_request_postmessageschannel},
    {"getmessageschannel", &hb_grid_request_getmessageschannel},
    {"avatar", &hb_grid_request_avatar},
    {"command", &hb_grid_request_command},
    {"neweventstopic", &hb_grid_request_neweventstopic},
    {"geteventstopic", &hb_grid_request_geteventstopic},
};
//////////////////////////////////////////////////////////////////////////
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
        evhttp_send_reply( _request, HTTP_OK, "", output_buffer );

        return;
    }

    hb_grid_process_handle_t * process = (hb_grid_process_handle_t *)_ud;

    char cmd_name[64 + 1];
    int32_t count = sscanf( uri, "/%64[^'/']", cmd_name );

    if( count != 1 )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "cmd count != 1", output_buffer );

        return;
    }

    if( hb_http_is_request_json( _request ) == HB_FALSE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "request content type should be of json type", output_buffer );

        return;
    }

    hb_json_handle_t * json_data_handle;
    if( hb_http_get_request_json( _request, &json_data_handle ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "bad json format", output_buffer );

        return;
    }

#ifdef HB_DEBUG
    char json_string[HB_DATA_MAX_SIZE];
    hb_size_t json_string_size;
    if( hb_json_dumps( json_data_handle, json_string, HB_DATA_MAX_SIZE, &json_string_size ) == HB_FAILURE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "bad json dumps", output_buffer );

        return;
    }

    HB_LOG_MESSAGE_INFO( "grid", "request '%s' data: %.*s"
        , cmd_name
        , json_string_size
        , json_string
    );
#endif

    hb_http_code_t response_code = HTTP_OK;

    hb_grid_request_handle_t args;
    args.process = process;
    args.data = json_data_handle;
    strcpy( args.reason, "" );
    strcpy( args.response, "{}" );

    hb_bool_t cmd_found = HB_FALSE;

    for( const hb_grid_cmd_inittab_t
        * cmd_inittab = grid_cmds,
        *cmd_inittab_end = grid_cmds + sizeof( grid_cmds ) / sizeof( grid_cmds[0] );
        cmd_inittab != cmd_inittab_end;
        ++cmd_inittab )
    {
        if( strcmp( cmd_inittab->name, cmd_name ) != 0 )
        {
            continue;
        }

        response_code = (*cmd_inittab->request)(&args);

        cmd_found = HB_TRUE;

        break;
    }

    hb_json_destroy( json_data_handle );

    if( cmd_found == HB_FALSE )
    {
        evhttp_send_reply( _request, HTTP_NOTIMPLEMENTED, "command not found", output_buffer );

        return;
    }

    HB_LOG_MESSAGE_INFO( "grid", "response '%s' code: %d data: %s"
        , cmd_name
        , response_code
        , args.response
    );

    size_t response_size = strlen( args.response );

    evbuffer_add( output_buffer, args.response, response_size );

    struct evkeyvalq * output_headers = evhttp_request_get_output_headers( _request );

    evhttp_add_header( output_headers, "Access-Control-Allow-Origin", "*" );
    evhttp_add_header( output_headers, "Access-Control-Allow-Headers", "*" );
    evhttp_add_header( output_headers, "Access-Control-Allow-Methods", "POST" );
    evhttp_add_header( output_headers, "Content-Type", "application/json" );

    evhttp_send_reply( _request, response_code, args.reason, output_buffer );
}
//////////////////////////////////////////////////////////////////////////
static void __hb_ev_thread_base( void * _ud )
{
    HB_UNUSED( _ud );

    hb_grid_process_handle_t * handle = (hb_grid_process_handle_t *)_ud;

    hb_db_client_handle_t * db_client;
    if( hb_db_create_client( handle->db, &db_client ) == HB_FAILURE )
    {
        return;
    }

    handle->db_client = db_client;

    struct event_base * base = event_base_new();
    HB_UNUSED( base );

    struct evhttp * http_server = evhttp_new( base );
    HB_UNUSED( http_server );

    evhttp_set_allowed_methods( http_server, EVHTTP_REQ_POST | EVHTTP_REQ_OPTIONS );

    evhttp_set_gencb( http_server, &__hb_grid_request, handle );

    hb_mutex_lock( handle->mutex_ev_socket );

    if( *handle->ev_socket == -1 )
    {
        struct evhttp_bound_socket * bound_socket = evhttp_bind_socket_with_handle( http_server, handle->grid_uri, handle->grid_port );

        *handle->ev_socket = evhttp_bound_socket_get_fd( bound_socket );
    }
    else
    {
        evhttp_accept_socket( http_server, *handle->ev_socket );
    }

    hb_mutex_unlock( handle->mutex_ev_socket );

    event_base_dispatch( base );

    hb_db_destroy_client( handle->db, handle->db_client );

    evhttp_free( http_server );
}
//////////////////////////////////////////////////////////////////////////
static void * __hb_memory_alloc( hb_size_t _size, void * _ud )
{
    HB_UNUSED( _ud );

    void * ptr = malloc( _size );

    return ptr;
}
//////////////////////////////////////////////////////////////////////////
static void * __hb_memory_realloc( void * _ptr, hb_size_t _size, void * _ud )
{
    HB_UNUSED( _ud );

    void * ptr = realloc( _ptr, _size );

    return ptr;
}
//////////////////////////////////////////////////////////////////////////
static void __hb_memory_free( const void * _ptr, void * _ud )
{
    HB_UNUSED( _ud );

    free( (void *)_ptr );
}
//////////////////////////////////////////////////////////////////////////
int main( int _argc, char * _argv[] )
{
    HB_UNUSED( _argc );
    HB_UNUSED( _argv );

    hb_memory_initialize( &__hb_memory_alloc, &__hb_memory_realloc, &__hb_memory_free, HB_NULLPTR );

    hb_log_initialize();

    if( hb_log_console_initialize() == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

#ifdef HB_PLATFORM_WINDOWS
    const WORD wVersionRequested = MAKEWORD( 2, 2 );

    WSADATA wsaData;
    int32_t err = WSAStartup( wVersionRequested, &wsaData );

    if( err != 0 )
    {
        return EXIT_FAILURE;
    }
#endif

    const char * config_file = HB_NULLPTR;
    hb_getopt( _argc, _argv, "--config", &config_file );

    hb_grid_config_t * config = HB_NEW( hb_grid_config_t );

    uint32_t max_thread = 16;
    uint32_t factor_mutex = 4;

    char grid_uri[HB_MAX_URI];
    strcpy( grid_uri, "127.0.0.1" );
    uint16_t grid_port = 5555;

    strcpy( config->name, "hb" );
    strcpy( config->cache_uri, "127.0.0.1" );
    config->cache_port = 6379;
    config->cache_timeout = 2000;
    strcpy( config->db_uri, "" );
    strcpy( config->db_host, "127.0.0.1" );
    config->db_port = 27017;
    strcpy( config->log_uri, "127.0.0.1" );
    config->log_port = 5044;

#ifndef HB_DEBUG
    strcpy( config->log_file, "" );
#else
    hb_date_t date;
    hb_date( &date );

    sprintf( config->log_file, "log_grid_%u_%u_%u_%u_%u_%u.log"
        , date.year
        , date.mon
        , date.mday
        , date.hour
        , date.min
        , date.sec );
#endif

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

        hb_json_get_field_uint32( json_handle, "max_thread", &max_thread, max_thread );
        hb_json_get_field_uint32( json_handle, "factor_mutex", &factor_mutex, factor_mutex );
        hb_json_copy_field_string( json_handle, "grid_uri", grid_uri, HB_MAX_URI, grid_uri );
        hb_json_get_field_uint16( json_handle, "grid_port", &grid_port, grid_port );

        hb_json_copy_field_string( json_handle, "name", config->name, 32, config->name );
        hb_json_copy_field_string( json_handle, "cache_uri", config->cache_uri, HB_MAX_URI, config->cache_uri );
        hb_json_get_field_uint16( json_handle, "cache_port", &config->cache_port, config->cache_port );
        hb_json_get_field_uint16( json_handle, "cache_timeout", &config->cache_timeout, config->cache_timeout );        
        hb_json_copy_field_string( json_handle, "db_uri", config->db_uri, HB_MAX_URI, config->db_uri );
        hb_json_copy_field_string( json_handle, "db_host", config->db_host, 128, config->db_host );
        hb_json_get_field_uint16( json_handle, "db_port", &config->db_port, config->db_port );
        hb_json_copy_field_string( json_handle, "log_file", config->log_file, HB_MAX_PATH, config->log_file );
        hb_json_copy_field_string( json_handle, "log_uri", config->log_uri, HB_MAX_URI, config->log_uri );
        hb_json_get_field_uint16( json_handle, "log_port", &config->log_port, config->log_port );

        hb_json_destroy( json_handle );
    }

    if( strcmp( config->log_file, "" ) != 0 )
    {
        if( hb_log_file_initialize( config->log_file ) == HB_FAILURE )
        {
            HB_LOG_MESSAGE_WARNING( "grid", "grid '%s' invalid initialize [log] file '%s'"
                , config->name
                , config->log_file
            );
        }
    }

    if( strcmp( config->log_uri, "" ) != 0 )
    {
        if( hb_log_tcp_initialize( config->log_uri, config->log_port ) == HB_FAILURE )
        {
            HB_LOG_MESSAGE_WARNING( "grid", "grid '%s' invalid initialize [log] uri '%s'"
                , config->name
                , config->log_uri
            );
        }
    }

    HB_LOG_MESSAGE_INFO( "grid", "start grid with config:" );
    HB_LOG_MESSAGE_INFO( "grid", "------------------------------------" );
    HB_LOG_MESSAGE_INFO( "grid", "max_thread: %u", max_thread );
    HB_LOG_MESSAGE_INFO( "grid", "grid_uri: %s", grid_uri );
    HB_LOG_MESSAGE_INFO( "grid", "grid_port: %u", grid_port );
    HB_LOG_MESSAGE_INFO( "grid", "name: %s", config->name );
    HB_LOG_MESSAGE_INFO( "grid", "cache_uri: %s", config->cache_uri );
    HB_LOG_MESSAGE_INFO( "grid", "cache_port: %u", config->cache_port );
    HB_LOG_MESSAGE_INFO( "grid", "cache_timeout: %u", config->cache_timeout );    
    HB_LOG_MESSAGE_INFO( "grid", "db_uri: %s", config->db_uri );
    HB_LOG_MESSAGE_INFO( "grid", "db_host: %s", config->db_host );
    HB_LOG_MESSAGE_INFO( "grid", "db_port: %u", config->db_port );
    HB_LOG_MESSAGE_INFO( "grid", "log_uri: %s", config->log_uri );
    HB_LOG_MESSAGE_INFO( "grid", "log_port: %u", config->log_port );
    HB_LOG_MESSAGE_INFO( "grid", "------------------------------------" );

    HB_LOG_MESSAGE_INFO( "grid", "[db] try..." );

    hb_db_handle_t * db;
    if( hb_db_initialze( config->db_uri, config->db_host, config->db_port, &db ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "grid", "grid '%s' invalid initialize [db] component uri: %s host: %s port: %u"
            , config->name
            , config->db_uri
            , config->db_host
            , config->db_port
        );

        return EXIT_FAILURE;
    }

    HB_LOG_MESSAGE_INFO( "grid", "[db] created" );

    HB_LOG_MESSAGE_INFO( "grid", "[matching] try..." );

    hb_matching_handle_t * matching;
    if( hb_matching_create( 1024, &matching ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "grid", "grid '%s' invalid initialize [matching] component"
            , config->name
        );

        return EXIT_FAILURE;
    }

    HB_LOG_MESSAGE_INFO( "grid", "[matching] created" );

    HB_LOG_MESSAGE_INFO( "grid", "[messages] try..." );

    hb_messages_handle_t * messages;
    if( hb_messages_create( &messages ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "grid", "grid '%s' invalid initialize [messages] component"
            , config->name
        );

        return EXIT_FAILURE;
    }

    HB_LOG_MESSAGE_INFO( "grid", "[messages] created" );

    HB_LOG_MESSAGE_INFO( "grid", "[events] try..." );

    hb_events_handle_t * events;
    if( hb_events_create( &events ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "grid", "grid '%s' invalid initialize [events] component"
            , config->name
        );

        return EXIT_FAILURE;
    }

    HB_LOG_MESSAGE_INFO( "grid", "[events] created" );

    HB_LOG_MESSAGE_INFO( "grid", "[economics] try..." );

    hb_economics_handle_t * economics;
    if( hb_economics_create( &economics ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "grid", "grid '%s' invalid initialize [economics] component"
            , config->name
        );

        return EXIT_FAILURE;
    }

    HB_LOG_MESSAGE_INFO( "grid", "[economics] created" );

    uint32_t mutex_handles_count = max_thread * factor_mutex;

    hb_grid_mutex_handle_t * mutex_handles = HB_NEWN( hb_grid_mutex_handle_t, mutex_handles_count );

    for( uint32_t i = 0; i != mutex_handles_count; ++i )
    {
        hb_grid_mutex_handle_t * mutex_handle = mutex_handles + i;

        if( hb_mutex_create( &mutex_handle->mutex ) == HB_FAILURE )
        {
            return EXIT_FAILURE;
        }
    }

    hb_mutex_handle_t * mutex_ev_socket;

    if( hb_mutex_create( &mutex_ev_socket ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_grid_process_handle_t * process_handles = HB_NEWN( hb_grid_process_handle_t, max_thread );

    evutil_socket_t ev_socket = -1;
    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_grid_process_handle_t * process_handle = process_handles + i;

        strcpy( process_handle->grid_uri, grid_uri );
        process_handle->grid_port = grid_port;

        process_handle->ev_socket = &ev_socket;
        process_handle->mutex_ev_socket = mutex_ev_socket;

        process_handle->db = db;
        process_handle->config = config;
        process_handle->matching = matching;
        process_handle->messages = messages;
        process_handle->events = events;
        process_handle->economics = economics;

        process_handle->mutex_handles = mutex_handles;
        process_handle->mutex_count = max_thread * factor_mutex;

        process_handle->cache = HB_NULLPTR;
        process_handle->thread = HB_NULLPTR;

        HB_LOG_MESSAGE_INFO( "grid", "[cache] try... (%u)", i );

        hb_cache_handle_t * cache;
        if( hb_cache_create( config->cache_uri, config->cache_port, config->cache_timeout, &cache ) == HB_FAILURE )
        {
            HB_LOG_MESSAGE_ERROR( "grid", "grid '%s' invalid create [cache] component [uri %s:%u]"
                , config->name
                , config->cache_uri
                , config->cache_port
            );

            return EXIT_FAILURE;
        }

        process_handle->cache = cache;

        HB_LOG_MESSAGE_INFO( "grid", "[cache] created (%u)", i );

        HB_LOG_MESSAGE_INFO( "grid", "[thread] try... (%u)", i );

        hb_thread_handle_t * thread;
        if( hb_thread_create( &__hb_ev_thread_base, process_handle, &thread ) == HB_FAILURE )
        {
            HB_LOG_MESSAGE_ERROR( "grid", "grid '%s' invalid create thread"
                , config->name
            );

            return EXIT_FAILURE;
        }

        process_handle->thread = thread;

        HB_LOG_MESSAGE_INFO( "grid", "[thread] created (%u)", i );
    }

    HB_LOG_MESSAGE_INFO( "grid", "ready.." );

    HB_LOG_MESSAGE_INFO( "grid", "------------------------------------" );

    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_grid_process_handle_t * process_handle = process_handles + i;

        if( process_handle->thread != HB_NULLPTR )
        {
            hb_thread_join( process_handle->thread );
        }
    }

    hb_mutex_destroy( mutex_ev_socket );
    mutex_ev_socket = HB_NULLPTR;

    for( uint32_t i = 0; i != mutex_handles_count; ++i )
    {
        hb_grid_mutex_handle_t * mutex_handle = mutex_handles + i;

        hb_mutex_destroy( mutex_handle->mutex );
    }


    for( uint32_t i = 0; i != max_thread; ++i )
    {
        hb_grid_process_handle_t * process_handle = process_handles + i;

        if( process_handle->cache != HB_NULLPTR )
        {
            hb_cache_destroy( process_handle->cache );
        }

        if( process_handle->thread != HB_NULLPTR )
        {
            hb_thread_destroy( process_handle->thread );
        }
    }

    HB_DELETEN( process_handles );

    hb_messages_destroy( messages );
    hb_matching_destroy( matching );
    hb_events_destroy( events );
    hb_economics_destroy( economics );

    hb_db_finalize( db );

    hb_log_tcp_finalize();

    HB_DELETE( config );

#ifdef HB_PLATFORM_WINDOWS
    WSACleanup();
#endif

    hb_log_console_finalize();

#ifdef HB_DEBUG
    hb_log_file_finalize();
#endif

    hb_log_finalize();

    return EXIT_SUCCESS;
}