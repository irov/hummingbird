#ifndef HB_CONFIG_H_
#define HB_CONFIG_H_

#define HB_VERSION_MAJOR 0
#define HB_VERSION_MINOR 1
#define HB_VERSION_PATCH 0

#include <stdint.h>
#include <stddef.h>

typedef uint32_t hb_bool_t;

static const hb_bool_t HB_TRUE = 1;
static const hb_bool_t HB_FALSE = 0;

typedef enum hb_result_e
{
    HB_SUCCESSFUL,
    HB_FAILURE,
} hb_result_e;

typedef enum hb_error_code_e
{
    HB_ERROR_INTERNAL,
    HB_ERROR_NOT_FOUND,
} hb_error_code_e;

typedef hb_result_e hb_result_t;

typedef uint32_t hb_pid_t;
typedef char hb_pid16_t[8];
typedef uint8_t hb_oid_t[12];
typedef uint8_t hb_sha1_t[20];
typedef uint8_t hb_token_t[14];
typedef char hb_token16_t[28];

typedef struct hb_token_handle_t
{
    hb_oid_t uoid;
    hb_oid_t poid;
} hb_token_handle_t;

#ifndef HB_DATA_MAX_SIZE
#define HB_DATA_MAX_SIZE 10240
#endif

typedef char hb_source_t[HB_DATA_MAX_SIZE];
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
#define HB_MAX_PATH 260
#endif

#ifndef HB_GRID_REQUEST_DATA_MAX_SIZE
#define HB_GRID_REQUEST_DATA_MAX_SIZE 2048
#endif

#ifndef HB_SHAREDMEMORY_SIZE
#define HB_SHAREDMEMORY_SIZE 65536
#endif

#endif
