#ifndef HB_RING_H_
#define HB_RING_H_

#include "hb_config/hb_config.h"

#define HB_RING_NEXT( PREFIX ) HB_PP_CONCATENATE(PREFIX, __ring_next)
#define HB_RING_PREV( PREFIX ) HB_PP_CONCATENATE(PREFIX, __ring_prev)

#define HB_RING_DECLARE( PREFIX, TYPE ) \
    struct TYPE * HB_RING_NEXT(PREFIX); \
    struct TYPE * HB_RING_PREV(PREFIX)

#define HB_RING_INIT( PREFIX, HEAD ) \
    do { \
        (HEAD)->HB_RING_NEXT(PREFIX) = (HEAD); \
        (HEAD)->HB_RING_PREV(PREFIX) = (HEAD); \
    } while(0)

#define HB_RING_IS_EMPTY( PREFIX, HEAD ) \
    ((HEAD)->HB_RING_NEXT(PREFIX) == (HEAD))

#define HB_RING_PUSH_BACK( PREFIX, HEAD, NODE ) \
    do { \
        (NODE)->HB_RING_NEXT(PREFIX) = (HEAD); \
        (NODE)->HB_RING_PREV(PREFIX) = (HEAD)->HB_RING_PREV(PREFIX); \
        (HEAD)->HB_RING_PREV(PREFIX)->HB_RING_NEXT(PREFIX) = (NODE); \
        (HEAD)->HB_RING_PREV(PREFIX) = (NODE); \
    } while(0)

#define HB_RING_PUSH_FRONT( PREFIX, HEAD, NODE ) \
    do { \
        (NODE)->HB_RING_NEXT(PREFIX) = (HEAD)->HB_RING_NEXT(PREFIX); \
        (NODE)->HB_RING_PREV(PREFIX) = (HEAD); \
        (HEAD)->HB_RING_NEXT(PREFIX)->HB_RING_PREV(PREFIX) = (NODE); \
        (HEAD)->HB_RING_NEXT(PREFIX) = (NODE); \
    } while(0)

#define HB_RING_POP_BACK( PREFIX, HEAD ) \
    do { \
        (HEAD)->HB_RING_PREV(PREFIX)->HB_RING_PREV(PREFIX)->HB_RING_NEXT(PREFIX) = (HEAD); \
        (HEAD)->HB_RING_PREV(PREFIX) = (HEAD)->HB_RING_PREV(PREFIX)->HB_RING_PREV(PREFIX); \
    } while(0)

#define HB_RING_POP_FRONT( PREFIX, HEAD ) \
    do { \
        (HEAD)->HB_RING_NEXT(PREFIX)->HB_RING_NEXT(PREFIX)->HB_RING_PREV(PREFIX) = (HEAD); \
        (HEAD)->HB_RING_NEXT(PREFIX) = (HEAD)->HB_RING_NEXT(PREFIX)->HB_RING_NEXT(PREFIX); \
    } while(0)

#define HB_RING_GET_NEXT( PREFIX, NODE ) \
    (NODE)->HB_RING_NEXT(PREFIX)

#define HB_RING_GET_PREV( PREFIX, NODE ) \
    (NODE)->HB_RING_PREV(PREFIX)

#define HB_RING_GET_PREV_PREV( PREFIX, NODE ) \
    (NODE)->HB_RING_PREV(PREFIX)->HB_RING_PREV(PREFIX)

#define HB_RING_POP_PREV( PREFIX, NODE ) \
    do { \
        (NODE)->HB_RING_PREV(PREFIX)->HB_RING_PREV(PREFIX)->HB_RING_NEXT(PREFIX) = (NODE); \
        (NODE)->HB_RING_PREV(PREFIX) = (NODE)->HB_RING_PREV(PREFIX)->HB_RING_PREV(PREFIX); \
    } while(0)

#define HB_RING_POP_NEXT( PREFIX, NODE ) \
    do { \
        (NODE)->HB_RING_NEXT(PREFIX)->HB_RING_NEXT(PREFIX)->HB_RING_PREV(PREFIX) = (NODE); \
        (NODE)->HB_RING_NEXT(PREFIX) = (NODE)->HB_RING_NEXT(PREFIX)->HB_RING_NEXT(PREFIX); \
    } while(0)

#define HB_RING_REMOVE( PREFIX, NODE ) \
    do { \
        (NODE)->HB_RING_PREV(PREFIX)->HB_RING_NEXT(PREFIX) = (NODE)->HB_RING_NEXT(PREFIX); \
        (NODE)->HB_RING_NEXT(PREFIX)->HB_RING_PREV(PREFIX) = (NODE)->HB_RING_PREV(PREFIX); \
    } while(0)

#define HB_RING_FOREACH( PREFIX, TYPE, NODE, ITERATOR ) \
    for( TYPE * ITERATOR = (NODE), * ITERATOR##__foreach_next = HB_NULLPTR; \
        ITERATOR##__foreach_next != (NODE); \
        ITERATOR##__foreach_next = ITERATOR = HB_RING_GET_NEXT(PREFIX, ITERATOR) )

#endif