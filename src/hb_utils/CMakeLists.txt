PROJECT(hb_utils C)

ADD_FILTER(
src
	hb_sha1.h
	hb_sha1.c
    
    hb_getopt.h
    hb_getopt.c
    
    hb_httpopt.h
    hb_httpopt.c
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${HUMMINGBIRD_SOURCE_FILES})