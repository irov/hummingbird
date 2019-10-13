PROJECT(hb_utils C)

ADD_FILTER(
src
	hb_sha1.h
	hb_sha1.c
    
    hb_base64.h
    hb_base64.c
    
    hb_getopt.h
    hb_getopt.c
    
    hb_httpopt.h
    hb_httpopt.c
    
    hb_memmem.h
    hb_memmem.c
    
    hb_multipart.h
    hb_multipart.c

	hb_strstre.h
	hb_strstre.c
    
    hb_time.h
    hb_time.c
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${HUMMINGBIRD_SOURCE_FILES})