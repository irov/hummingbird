#include "hb_node.h"

#include "hb_db/hb_db.h"
#include "hb_script/hb_script.h"
#include "hb_storage/hb_storage.h"
#include "hb_file/hb_file.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main( int _argc, const char * _argv[] )
{
    HB_UNUSED( _argc );
    HB_UNUSED( _argv );

    hb_db_initialze( "hb_grid", "mongodb://localhost:27017" );
    hb_storage_initialize( "$user_id$", "hb_storage", "hb_files" );
    hb_file_initialize( ".store/" );

    hb_script_initialize( "5d932e6820cdb53b7c26b73f" );

    FILE * f = fopen( "server.lua", "rb" );
    fseek( f, 0L, SEEK_END );
    long sz = ftell( f );
    rewind( f );

    char fbuf[2048];
    fread( fbuf, sz, 1, f );
    fclose( f );

    uint8_t sha1[20];
    if( hb_storage_set( fbuf, sz, sha1 ) == 0 )
    {
        return EXIT_FAILURE;
    }

    size_t fbufl;
    char fbufg[10240];
    if( hb_storage_get( sha1, fbufg, 10240, &fbufl ) == 0 )
    {
        return EXIT_FAILURE;
    }

    if( hb_script_load( fbufg, fbufl ) == 0 )
    {
        return EXIT_FAILURE;
    }

    char res[1024];
    if( hb_script_call( "test", "return {test=17}", strlen( "return {test=17}" ), res, 1024 ) == 0 )
    {
        return EXIT_FAILURE;
    }

    hb_script_finalize();

    return EXIT_SUCCESS;
}