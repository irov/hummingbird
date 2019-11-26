#ifndef HB_FILE_H_
#define HB_FILE_H_

#include "hb_config/hb_config.h"

typedef struct hb_file_handle_t hb_file_handle_t;

hb_result_t hb_file_initialize( const char * _folder );
void hb_file_finalize();

hb_result_t hb_file_available();

hb_result_t hb_file_open_read( const char * _path, hb_file_handle_t * _handle );
hb_result_t hb_file_open_write( const char * _path, hb_file_handle_t * _handle );
hb_result_t hb_file_read( hb_file_handle_t * _handle, void * _buffer, size_t _capacity );
hb_result_t hb_file_write( hb_file_handle_t * _handle, const void * _buffer, size_t _size );
void hb_file_close( hb_file_handle_t * _handle );

#endif
