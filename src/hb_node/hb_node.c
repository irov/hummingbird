#include "hb_node.h"

#include "hb_db/hb_db.h"
#include "hb_script/hb_script.h"

#include <stdlib.h>
#include <stdio.h>

int main( int _argc, const char * _argv[] )
{
    HB_UNUSED( _argc );
    HB_UNUSED( _argv );

    hb_db_initialze( "hb_grid", "mongodb://localhost:27017" );

    hb_script_initialize( "5d932e6820cdb53b7c26b73f" );

    const char s[] = "a, b = server.GetCurrentUserData({\"a\", \"b\"}); print(a); print(b)";

    hb_script_load( s, sizeof( s ) - 1 );

    hb_script_finalize();


    return EXIT_SUCCESS;
}