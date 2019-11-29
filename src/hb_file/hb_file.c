#include "hb_file.h"

#include "hb_log/hb_log.h"

#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <direct.h>

//////////////////////////////////////////////////////////////////////////
#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif
//////////////////////////////////////////////////////////////////////////
typedef struct hb_file_handle_t
{
    char path[HB_MAX_PATH];
    FILE * f;
    size_t size;
} hb_file_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_file_settings_t
{
    char folder[256];

} hb_file_settings_t;
//////////////////////////////////////////////////////////////////////////
static hb_file_settings_t * g_file_settings;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_file_initialize( const char * _folder )
{
    g_file_settings = HB_NEW( hb_file_settings_t );
    strcpy( g_file_settings->folder, _folder );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_file_finalize()
{
    HB_DELETE( g_file_settings );
    g_file_settings = HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_file_available()
{
    if( g_file_settings == HB_NULLPTR )
    {
        return HB_FALSE;
    }

    if( g_file_settings->folder[0] == '\0' )
    {
        return HB_FALSE;
    }

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_file_open_read( const char * _path, hb_file_handle_t ** _handle )
{
    char fullpath[HB_MAX_PATH];
    strcpy( fullpath, g_file_settings->folder );
    strncat( fullpath, _path, 2 );
    strcat( fullpath, "/" );
    strncat( fullpath, _path + 2, 2 );
    strcat( fullpath, "/" );
    strcat( fullpath, _path + 4 );

    FILE * f = fopen( fullpath, "rb" );

    if( f == NULL )
    {
        return HB_FAILURE;
    }

    hb_file_handle_t * handle = HB_NEW( hb_file_handle_t );

    strcpy( handle->path, _path );
    handle->f = f;

    fseek( f, 0L, SEEK_END );
    long sz = ftell( f );
    rewind( f );

    handle->size = (size_t)sz;

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_file_make_directory( const char * _path )
{
    struct stat sb;
    if( stat( _path, &sb ) == 0 && S_ISREG( sb.st_mode ) )
    {
        return HB_FAILURE;
    }

    _mkdir( _path );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_file_open_write( const char * _path, hb_file_handle_t * _handle )
{
    char fullpath[256];
    strcpy( fullpath, g_file_settings->folder );

    __hb_file_make_directory( fullpath );

    strncat( fullpath, _path, 2 );
    strcat( fullpath, "/" );

    __hb_file_make_directory( fullpath );

    strncat( fullpath, _path + 2, 2 );
    strcat( fullpath, "/" );

    __hb_file_make_directory( fullpath );

    strcat( fullpath, _path + 4 );

    FILE * f = fopen( fullpath, "wb" );

    if( f == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    strcpy( _handle->path, _path );
    _handle->f = f;
    _handle->size = 0;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_file_read( hb_file_handle_t * _handle, void * _buffer, size_t _capacity )
{
    FILE * f = _handle->f;
    size_t sz = _handle->size;

    if( sz == 0 )
    {
        return HB_FAILURE;
    }

    if( _capacity < sz )
    {
        return HB_FAILURE;
    }

    size_t r = fread( _buffer, sz, 1, f );

    if( r != 1 )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_file_write( hb_file_handle_t * _handle, const void * _buffer, size_t _size )
{
    FILE * f = _handle->f;

    size_t r = fwrite( _buffer, _size, 1, f );

    if( r != 1 )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
size_t hb_file_get_size( hb_file_handle_t * _handle )
{
    return _handle->size;
}
//////////////////////////////////////////////////////////////////////////
void hb_file_close( hb_file_handle_t * _handle )
{
    FILE * f = _handle->f;

    int res = fclose( f );

    if( res != 0 )
    {
        HB_LOG_MESSAGE_WARNING( "file", "invalid close file '%s' error [%d]"
            , _handle->path
            , res
        );
    }

    HB_DELETE( _handle );
}
