#include "hb_http.h"

#include "hb_utils/hb_strstre.h"

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_http_get_request_params( struct evhttp_request * _request, multipart_params_handle_t * _params, uint32_t _capacity, uint32_t * _count )
{
    enum evhttp_cmd_type command_type = evhttp_request_get_command( _request );
    HB_UNUSED( command_type );

    struct evkeyvalq * headers = evhttp_request_get_input_headers( _request );
    HB_UNUSED( headers );

    const char * content_type = evhttp_find_header( headers, "Content-Type" );
    HB_UNUSED( content_type );

    if( content_type == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    const char * content_type_boundary = hb_strstre( content_type, "boundary=" );

    char boundary[64];
    int boundary_size = sprintf( boundary, "--%s", content_type_boundary );

    struct evbuffer * input_buffer = evhttp_request_get_input_buffer( _request );

    size_t multipart_length = evbuffer_get_length( input_buffer );

    if( multipart_length + 1 > HB_DATA_MAX_SIZE )
    {
        return HB_FAILURE;
    }

    hb_data_t multipart;
    ev_ssize_t copyout_buffer_size = evbuffer_copyout( input_buffer, multipart, multipart_length );
    HB_UNUSED( copyout_buffer_size );

    ((char *)multipart)[copyout_buffer_size] = '\0';

    if( hb_multipart_parse( boundary, boundary_size, multipart, multipart_length, _params, _capacity, _count ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_http_get_request_data( struct evhttp_request * _request, void * _buffer, size_t _capacity, size_t * _size )
{
    struct evbuffer * input_buffer = evhttp_request_get_input_buffer( _request );

    size_t multipart_length = evbuffer_get_length( input_buffer );

    if( multipart_length + 1 > _capacity )
    {
        return HB_FAILURE;
    }

    ev_ssize_t copyout_buffer_size = evbuffer_copyout( input_buffer, _buffer, multipart_length );

    if( copyout_buffer_size < 0 )
    {
        return HB_FAILURE;
    }

    ((char *)_buffer)[copyout_buffer_size] = '\0';

    *_size = copyout_buffer_size;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_http_is_request_json( struct evhttp_request * _request )
{
    struct evkeyvalq * headers = evhttp_request_get_input_headers( _request );

    const char * content_type = evhttp_find_header( headers, "Content-Type" );

    if( content_type == HB_NULLPTR )
    {
        return HB_FALSE;
    }

    if( strcmp( content_type, "application/json" ) != 0 )
    {
        return HB_FALSE;
    }

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_http_get_request_json( struct evhttp_request * _request, hb_json_handle_t ** _handle )
{
    if( hb_http_is_request_json( _request ) == HB_FALSE )
    {
        return HB_FAILURE;
    }

    hb_data_t data;
    size_t data_size;
    if( hb_http_get_request_data( _request, data, HB_DATA_MAX_SIZE, &data_size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_json_create( data, data_size, _handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_http_get_request_header( struct evhttp_request * _request, const char * _header, const char ** _value )
{
    enum evhttp_cmd_type command_type = evhttp_request_get_command( _request );
    HB_UNUSED( command_type );

    struct evkeyvalq * headers = evhttp_request_get_input_headers( _request );

    const char * value = evhttp_find_header( headers, _header );

    if( value == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    *_value = value;

    return HB_SUCCESSFUL;
}