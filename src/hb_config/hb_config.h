#ifndef HB_CONFIG_H_
#define HB_CONFIG_H_

#include <stdint.h>

#define HB_NULLPTR (0)

void * hb_new( size_t _size );
void hb_free( void * _ptr );

#define HB_NEW(TYPE) hb_new(sizeof(TYPE));
#define HB_DELETE(P) hb_free(p);

#endif
