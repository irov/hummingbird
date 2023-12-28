#ifndef HB_MEMORY_H_
#define HB_MEMORY_H_

#include "hb_config/hb_config.h"

typedef void * (*hb_memory_alloc_t)(hb_size_t _size, void * _ud);
typedef void * (*hb_memory_realloc_t)(void * _ptr, hb_size_t _size, void * _ud);
typedef void(*hb_memory_free_t)(const void * _ptr, void * _ud);

void hb_memory_initialize( hb_memory_alloc_t _alloc, hb_memory_realloc_t _realloc, hb_memory_free_t _free, void * _ud );

void * hb_memory_alloc( hb_size_t _size );
void * hb_memory_realloc( void * _ptr, hb_size_t _size );
void hb_memory_free( const void * _ptr );

#ifndef HB_ALLOC
#define HB_ALLOC(S) hb_memory_alloc((S))
#endif

#ifndef HB_REALLOC
#define HB_REALLOC(P, S) hb_memory_realloc((P), (S))
#endif

#ifndef HB_FREE
#define HB_FREE(P) hb_memory_free((P))
#endif

#ifndef HB_NEW
#define HB_NEW(TYPE) ((TYPE*)hb_memory_alloc(sizeof(TYPE)))
#endif

#ifndef HB_NEWE
#define HB_NEWE(TYPE, E) ((TYPE*)hb_memory_alloc(sizeof(TYPE) + E))
#endif

#ifndef HB_NEWN
#define HB_NEWN(TYPE, N) ((TYPE*)hb_memory_alloc(sizeof(TYPE) * N))
#endif

#ifndef HB_DELETE
#define HB_DELETE(P) hb_memory_free((P))
#endif

#ifndef HB_DELETEN
#define HB_DELETEN(P) hb_memory_free((P))
#endif

#endif
