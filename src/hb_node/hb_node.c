#include "hb_node.h"

#include "hb_db/hb_db.h"
#include "hb_script/hb_script.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main( int _argc, const char * _argv[] )
{
    HB_UNUSED( _argc );
    HB_UNUSED( _argv );

    hb_db_initialze( "hb_grid", "mongodb://localhost:27017" );

    hb_script_initialize( "5d932e6820cdb53b7c26b73f" );

    FILE * f = fopen( "server.lua", "rb" );
    fseek( f, 0L, SEEK_END );
    long sz = ftell( f );
    rewind( f );

    char fbuf[2048];
    fread( fbuf, sz, 1, f );
    fclose( f );

    hb_script_load( fbuf, sz );

    char res[1024];
    hb_script_call( "test", "return {test=17}", strlen( "return {test=17}" ), res, 1024 );

    hb_script_finalize();


    return EXIT_SUCCESS;
}