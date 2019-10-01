#include "hb_grid.h"

#include "hb_db/hb_db.h"

#include <stdlib.h>
#include <stdio.h>

int main( int _argc, const char * _argv[] )
{
    HB_UNUSED( _argc );
    HB_UNUSED( _argv );

    hb_db_initialze( "hb_grid", "mongodb://localhost:27017" );

    hb_db_collection_handler_t collection;
    hb_db_get_collection( "admin", "users", &collection );

    hb_db_value_handler_t value;
    hb_db_get_value( &collection, "5d90b689a1fb944214b3c899", HB_NULLPTR, 0, &value );

    const char * str = value.value[0];
    printf( "%s\n", str );

    hb_db_value_destroy( &value );

    hb_db_finalize();

    return EXIT_SUCCESS;
}