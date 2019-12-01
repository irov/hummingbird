PROJECT(hb_utils C)

IF(WIN32)
    SET(hb_time_platform hb_time_win32.c)
ELSEIF(UNIX)
    SET(hb_time_platform hb_time_unix.c)
ENDIF()


ADD_FILTER(
src
	hb_sha1.h
	hb_sha1.c
    
    hb_rand.h
    hb_rand.c
    
    hb_base16.h
    hb_base16.c
    
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
    ${hb_time_platform}
    
    hb_clock.h
    hb_clock.c

    hb_date.h
    hb_date.c
    
    hb_oid.h
    hb_oid.c
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${HUMMINGBIRD_SOURCE_FILES})