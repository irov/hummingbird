#include "hb_grid.h"

#include "hb_mongo/hb_mongo.h"

#include <stdlib.h>
#include <stdio.h>

#define HB_UNUSED(X) (void)(X)

int main( int _argc, const char * _argv[] )
{
    HB_UNUSED( _argc );
    HB_UNUSED( _argv );

    hb_mongo_initialze( "hb_grid", "mongodb://localhost:27017" );

    hb_mongo_collection_handler_t collection;
    hb_mongo_get_collection( "admin", "users", &collection );

    hb_mongo_value_handler_t value;
    hb_mongo_get_value( &collection, "5d90b689a1fb944214b3c899", &value );

    const char * str = value.value;
    printf( "%s\n", str );

    hb_mongo_value_destroy( &value );

    hb_mongo_finalize();

    return EXIT_SUCCESS;
}