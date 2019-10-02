#include "hb_storage.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_archive/hb_archive.h"
#include "hb_file/hb_file.h"
#include "hb_utils/hb_sha1.h"

#include <string.h>

#ifndef HB_STORAGE_MAX_SIZE
#define HB_STORAGE_MAX_SIZE 10240
#endif

//////////////////////////////////////////////////////////////////////////
typedef struct hb_storage_settings_t
{
    char user[25];

    hb_db_collection_handler_t db_collection;
} hb_storage_settings_t;
//////////////////////////////////////////////////////////////////////////
static hb_storage_settings_t * g_storage_settings;
//////////////////////////////////////////////////////////////////////////
int hb_storage_initialize( const char * _user, const char * _db, const char * _collection )
{
    g_storage_settings = HB_NEW( hb_storage_settings_t );
    strcpy( g_storage_settings->user, _user );

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
int hb_storage_set( const void * _data, size_t _size, uint8_t * _sha1 )
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

    hb_sha1( buffer, compressSize, _sha1 );

    if( hb_db_upload_file( &g_storage_settings->db_collection, _sha1, buffer, compressSize ) == 0 )
    {
        return 0;
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_storage_get( const uint8_t * _sha1, void * _data, size_t _capacity, size_t * _size )
{
    char sha1hex[41];

    int file_available = hb_file_available();

    if( file_available == 1 )
    {        
        hb_sha1_hex( _sha1, sha1hex );

        hb_file_handler_t read_file_handler;
        if( hb_file_open_read( sha1hex, &read_file_handler ) == 1 )
        {
            uint8_t compress_data[HB_STORAGE_MAX_SIZE];
            if( hb_file_read( &read_file_handler, compress_data, _capacity ) == 0 )
            {
                hb_file_close( &read_file_handler );

                return 0;
            }

            hb_file_close( &read_file_handler );

            hb_archive_decompress( _data, _capacity, compress_data, read_file_handler.length, _size );            

            return 1;
        }
    }

    hb_db_file_handler_t db_file_handler;
    if( hb_db_load_file( &g_storage_settings->db_collection, _sha1, &db_file_handler ) == 0 )
    {
        return 0;
    }

    if( file_available )
    {
        hb_file_handler_t write_file_handler;
        if( hb_file_open_write( sha1hex, &write_file_handler ) == 1 )
        {
            if( hb_file_write( &write_file_handler, db_file_handler.buffer, db_file_handler.length ) == 0 )
            {
                hb_file_close( &write_file_handler );

                return 0;
            }

            hb_file_close( &write_file_handler );
        }
    }

    hb_archive_decompress( _data, _capacity, db_file_handler.buffer, db_file_handler.length, _size );

    hb_db_close_file( &db_file_handler );

    return 1;
}
//////////////////////////////////////////////////////////////////////////