#ifndef HB_CONFIG_H_
#define HB_CONFIG_H_

#include <stdint.h>
#include <stddef.h>

#define HB_VERSION_MAJOR 0
#define HB_VERSION_MINOR 1
#define HB_VERSION_PATCH 0

#ifndef NDEBUG
#define HB_DEBUG
#endif

typedef enum hb_bool_e
{
    HB_FALSE,
    HB_TRUE,
} hb_bool_e;

typedef uint32_t hb_bool_t;

static const size_t HB_UNKNOWN_STRING_SIZE = (~0U);

typedef enum hb_result_e
{
    HB_SUCCESSFUL,
    HB_FAILURE,
} hb_result_e;

typedef enum hb_error_code_e
{
    HB_ERROR_OK,
    HB_ERROR_INTERNAL,
    HB_ERROR_NOT_FOUND,
} hb_error_code_e;

typedef hb_result_e hb_result_t;
typedef hb_error_code_e hb_error_code_t;

typedef uint8_t hb_byte_t;
typedef uint32_t hb_uid_t;

typedef struct hb_uid16_t
{
    char value[8];
} hb_uid16_t;

typedef struct hb_oid_t
{
    hb_byte_t value[12];
} hb_oid_t;

typedef struct hb_oid16_t
{
    char value[24];
} hb_oid16_t;

typedef struct hb_sha1_t
{
    hb_byte_t value[20];
} hb_sha1_t;

typedef struct hb_token_t
{
    hb_byte_t value[14];
} hb_token_t;

typedef struct hb_token16_t
{
    char value[28];
} hb_token16_t;

static const hb_oid_t HB_OID_NONE = { 0 };

typedef struct hb_user_token_t
{
    hb_oid_t uoid;
    hb_oid_t poid;
} hb_user_token_t;

typedef struct hb_account_token_t
{
    hb_oid_t aoid;
} hb_account_token_t;

#ifndef HB_DATA_MAX_SIZE
#define HB_DATA_MAX_SIZE 10240
#endif

typedef char hb_source_t[HB_DATA_MAX_SIZE];
typedef hb_byte_t hb_data_t[HB_DATA_MAX_SIZE];

#ifndef HB_UNUSED
#define HB_UNUSED(X) (void)(X)
#endif

#ifndef HB_NULLPTR
#define HB_NULLPTR ((void*)0)
#endif

#ifndef HB_MAX_PATH
#define HB_MAX_PATH 260
#endif

#ifndef HB_GRID_REQUEST_DATA_MAX_SIZE
#define HB_GRID_REQUEST_DATA_MAX_SIZE 2048
#endif

#if defined(WIN32)
#   define HB_PLATFORM_WINDOWS
#elif defined(__linux__) && !defined(__ANDROID__)
#   define HB_PLATFORM_LINUX
#else
#   error "undefine platform"
#endif

#endif
