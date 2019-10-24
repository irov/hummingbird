#ifndef HB_CONFIG_H_
#define HB_CONFIG_H_

#include <stdint.h>

typedef enum hb_result_e
{
    HB_FAILURE,
    HB_SUCCESSFUL,
} hb_result_e;

typedef hb_result_e hb_result_t;

typedef uint8_t hb_oid_t[12];
typedef uint8_t hb_sha1_t[20];
typedef uint8_t hb_token_t[12];

typedef struct hb_token_handle_t
{
    hb_oid_t user;
    hb_oid_t project;
} hb_token_handle_t;

#ifndef HB_DATA_MAX_SIZE
#define HB_DATA_MAX_SIZE 10240
#endif

typedef uint8_t hb_data_t[HB_DATA_MAX_SIZE];

#ifndef HB_UNUSED
#define HB_UNUSED(X) (void)(X)
#endif

#ifndef HB_NULLPTR
#define HB_NULLPTR ((void*)0)
#endif

void * hb_new( size_t _size );
void hb_free( void * _ptr );

#ifndef HB_NEW
#define HB_NEW(TYPE) ((TYPE*)hb_new(sizeof(TYPE)));
#endif

#ifndef HB_NEWN
#define HB_NEWN(TYPE, N) ((TYPE*)hb_new(sizeof(TYPE) * N));
#endif

#ifndef HB_DELETE
#define HB_DELETE(P) hb_free(P);
#endif

#ifndef HB_MAX_PATH
#define HB_MAX_PATH 250
#endif

#ifndef HB_GRID_REQUEST_MAX_SIZE
#define HB_GRID_REQUEST_MAX_SIZE 1024
#endif

#ifndef HB_MAGIC_NUMBER
#define HB_MAGIC_NUMBER(A,B,C,D) ((A << 0) + (B << 8) + (C << 16) + (D << 24))
#endif

#endif
