#include "hb_file.h"

#include "hb_config/hb_config.h"

#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <direct.h>

#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

//////////////////////////////////////////////////////////////////////////
typedef struct hb_file_settings_t
{
    char folder[256];

} hb_file_settings_t;
//////////////////////////////////////////////////////////////////////////
static hb_file_settings_t * g_file_settings;
//////////////////////////////////////////////////////////////////////////
int hb_file_initialize( const char * _folder )
{
    g_file_settings = HB_NEW( hb_file_settings_t );
    strcpy( g_file_settings->folder, _folder );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_file_finialize()
{
    HB_DELETE( g_file_settings );
    g_file_settings = HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
int hb_file_available()
{
    if( g_file_settings->folder[0] == '\0' )
    {
        return 0;
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_file_open_read( const char * _path, hb_file_handler_t * _handler )
{
    char fullpath[256];
    strcpy( fullpath, g_file_settings->folder );
    strncat( fullpath, _path, 2 );
    strcat( fullpath, "/" );
    strncat( fullpath, _path + 2, 2 );
    strcat( fullpath, "/" );
    strcat( fullpath, _path + 4 );

    FILE * f = fopen( fullpath, "rb" );

    if( f == NULL )
    {
        return 0;
    }

    _handler->handler = f;

    fseek( f, 0L, SEEK_END );
    long sz = ftell( f );
    rewind( f );

    _handler->length = (size_t)sz;    

    return 1;
}
//////////////////////////////////////////////////////////////////////////
static int __hb_file_make_directory( const char * _path )
{
    struct stat sb;
    if( stat( _path, &sb ) == 0 && S_ISREG( sb.st_mode ) )
    {
        return 0;
    }

    _mkdir( _path );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_file_open_write( const char * _path, hb_file_handler_t * _handler )
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

    if( f == NULL )
    {
        return 0;
    }

    _handler->handler = f;
    _handler->length = 0;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_file_read( hb_file_handler_t * _observer, void * _buffer, size_t _capacity )
{
    FILE * f = (FILE *)_observer->handler;
    size_t sz = _observer->length;

    if( sz == 0 )
    {
        return 0;
    }

    if( _capacity < sz )
    {
        return 0;
    }

    size_t r = fread( _buffer, sz, 1, f );

    if( r != 1 )
    {
        return 0;
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_file_write( hb_file_handler_t * _observer, const void * _buffer, size_t _size )
{
    FILE * f = (FILE *)_observer->handler;

    size_t r = fwrite( _buffer, _size, 1, f );

    if( r != 1 )
    {
        return 0;
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_file_close( hb_file_handler_t * _observer )
{
    FILE * f = (FILE *)_observer->handler;

    int res = fclose( f );

    if( res != 0 )
    {
        return 0;
    }

    return 1;
}
