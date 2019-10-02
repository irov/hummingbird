#include "hb_storage.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_archive/hb_archive.h"
#include "hb_utils/hb_sha1.h"

#include <string.h>

#ifndef HB_STORAGE_MAX_SIZE
#define HB_STORAGE_MAX_SIZE 10240
#endif

//////////////////////////////////////////////////////////////////////////
typedef struct hb_storage_settings_t
{
    char user[32];
    char folder[256];

    hb_db_collection_handler_t db_collection;
} hb_storage_settings_t;
//////////////////////////////////////////////////////////////////////////
static hb_storage_settings_t * g_storage_settings;
//////////////////////////////////////////////////////////////////////////
int hb_storage_initialize( const char * _user, const char * _db, const char * _collection, const char * _folder )
{
    g_storage_settings = HB_NEW( hb_storage_settings_t );
    strcpy( g_storage_settings->user, _user );
    strcpy( g_storage_settings->folder, _folder );

    if( hb_db_get_collection( _db, _collection, &g_storage_settings->db_collection ) == 0 )
    {
        return 0;
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_storage_finalize()
{
    hb_db_collection_destroy( &g_storage_settings->db_collection );
    
    HB_DELETE( g_storage_settings );
    g_storage_settings = HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
int hb_storage_set( const void * _data, size_t _size )
{
    size_t bound_size = hb_archive_bound( _size );

    if( bound_size >= HB_STORAGE_MAX_SIZE )
    {
        return 0;
    }

    size_t compressSize;

    uint8_t buffer[HB_STORAGE_MAX_SIZE];
    if( hb_archive_compress( buffer, HB_STORAGE_MAX_SIZE, _data, _size, &compressSize ) == 0 )
    {
        return 0;
    }

    uint8_t sha1[20];
    hb_sha1( buffer, compressSize, sha1 );

    hb_db_file_handler_t handler;
    hb_db_upload_file( &g_storage_settings->db_collection, sha1, buffer, compressSize, &handler );

    return 1;
}
