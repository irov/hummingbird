#include "hb_oid.h"

#include <string.h>

void hb_oid_copy( hb_oid_t _dst, const hb_oid_t _src )
{
    memcpy( _dst, _src, 12 );
}