#ifndef HB_ARCHIVE_H_
#define HB_ARCHIVE_H_

#include "hb_config/hb_config.h"

size_t hb_archive_bound( size_t _size );
hb_result_t hb_archive_compress( void * _buffer, size_t _capacity, const void * _source, size_t _size, size_t * _compressSize );
hb_result_t hb_archive_decompress( void * _buffer, size_t _capacity, const void * _source, size_t _size, size_t * _decompressSize );

#endif
