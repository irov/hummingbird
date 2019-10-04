#ifndef HB_CONFIG_H_
#define HB_CONFIG_H_

#include <stdint.h>

#define HB_UNUSED(X) (void)(X)
#define HB_NULLPTR ((void*)0)

void * hb_new( size_t _size );
void hb_free( void * _ptr );

#define HB_NEW(TYPE) ((TYPE*)hb_new(sizeof(TYPE)));
#define HB_NEWN(TYPE, N) ((TYPE*)hb_new(sizeof(TYPE) * N));
#define HB_DELETE(P) hb_free(P);

#endif
