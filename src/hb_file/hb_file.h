#ifndef HB_FILE_H_
#define HB_FILE_H_

#include "hb_config/hb_config.h"

int hb_file_initialize( const char * _folder );
void hb_file_finialize();

int hb_file_available();

typedef struct hb_file_handle_t
{
    void * handle;
    size_t length;

} hb_file_handle_t;

int hb_file_open_read( const char * _path, hb_file_handle_t * _handle );
int hb_file_open_write( const char * _path, hb_file_handle_t * _handle );
int hb_file_read( hb_file_handle_t * _handle, void * _buffer, size_t _capacity );
int hb_file_write( hb_file_handle_t * _handle, const void * _buffer, size_t _size );
int hb_file_close( hb_file_handle_t * _handle );

#endif
