#ifndef HB_ARCHIVE_H_
#define HB_ARCHIVE_H_

#include "hb_config/hb_config.h"

hb_size_t hb_archive_bound( hb_size_t _size );
hb_result_t hb_archive_compress( void * _buffer, hb_size_t _capacity, const void * _source, hb_size_t _size, hb_size_t * const _compressSize );
hb_result_t hb_archive_decompress( void * _buffer, hb_size_t _capacity, const void * _source, hb_size_t _size, hb_size_t * const _decompressSize );

#endif
