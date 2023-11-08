#include "hb_grid.h"

#include "hb_mutex/hb_mutex.h"
#include "hb_utils/hb_base16.h"

//////////////////////////////////////////////////////////////////////////
void hb_grid_process_lock( hb_grid_process_handle_t * _process, hb_uid_t _uuid )
{
    hb_grid_mutex_handle_t * mutex_handle = _process->mutex_handles + _uuid % _process->mutex_count;
    hb_mutex_lock( mutex_handle->mutex );
}
//////////////////////////////////////////////////////////////////////////
void hb_grid_process_unlock( hb_grid_process_handle_t * _process, hb_uid_t _uuid )
{
    hb_grid_mutex_handle_t * mutex_handle = _process->mutex_handles + _uuid % _process->mutex_count;
    hb_mutex_unlock( mutex_handle->mutex );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_get_account_token( hb_grid_request_handle_t * _handle, const char * _field, hb_account_token_t * const _token )
{
    hb_result_t result = hb_cache_get_token( _handle->process->cache, _field, 1800, _token, sizeof( hb_account_token_t ), HB_NULLPTR );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_get_user_token( hb_grid_request_handle_t * _handle, const char * _field, hb_user_token_t * const _token )
{
    hb_result_t result = hb_cache_get_token( _handle->process->cache, _field, 1800, _token, sizeof( hb_user_token_t ), HB_NULLPTR );
    
    return result;     
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_get_uid( const char * _base16, hb_uid_t * const _uid )
{
    hb_result_t result = hb_base16_decode( _base16, HB_UNKNOWN_STRING_SIZE, _uid, sizeof( hb_uid_t ), HB_NULLPTR );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_get_arg_json( hb_grid_request_handle_t * _handle, const char * _field, hb_json_handle_t ** _json )
{
    if( hb_json_get_field( _handle->data, _field, _json ) == HB_FAILURE )
    {
        snprintf( _handle->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get [json] field: %s", _field );

        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_get_arg_string( hb_grid_request_handle_t * _handle, const char * _field, const char ** _value )
{
    if( hb_json_get_field_string( _handle->data, _field, _value, HB_NULLPTR ) == HB_FAILURE )
    {
        snprintf( _handle->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get [string] field: %s", _field );

        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_get_arg_string_size( hb_grid_request_handle_t * _handle, const char * _field, const char ** _value, hb_size_t * const _size )
{
    if( hb_json_get_field_string( _handle->data, _field, _value, _size ) == HB_FAILURE )
    {
        snprintf( _handle->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get [string] field: %s", _field );

        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_get_arg_int32( hb_grid_request_handle_t * _handle, const char * _field, int32_t * const _value )
{
    if( hb_json_get_field_int32( _handle->data, _field, _value ) == HB_FAILURE )
    {
        snprintf( _handle->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get [int32] field: %s", _field );

        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_get_arg_uint32( hb_grid_request_handle_t * _handle, const char * _field, uint32_t * const _value )
{
    if( hb_json_get_field_uint32( _handle->data, _field, _value ) == HB_FAILURE )
    {
        snprintf( _handle->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get [uint32] field: %s", _field );

        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_get_arg_uint64( hb_grid_request_handle_t * _handle, const char * _field, uint64_t * const _value )
{
    if( hb_json_get_field_uint64( _handle->data, _field, _value ) == HB_FAILURE )
    {
        snprintf( _handle->reason, HB_GRID_REASON_DATA_MAX_SIZE, "invalid get [uint64] field: %s", _field );

        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////