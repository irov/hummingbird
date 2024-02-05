PROJECT(hb_utils LANGUAGES C)

IF(HUMMINGBIRD_TARGET_WIN32)
    SET(hb_time_platform hb_time_win32.c)
    SET(hb_sleep_platform hb_sleep_win32.c)
ELSEIF(HUMMINGBIRD_TARGET_UNIX)
    SET(hb_time_platform hb_time_unix.c)
    SET(hb_sleep_platform hb_sleep_unix.c)
ELSEIF(HUMMINGBIRD_TARGET_OSX)
    SET(hb_time_platform hb_time_macos.c)
    SET(hb_sleep_platform hb_sleep_macos.c)
ENDIF()

HUMMINGBIRD_ADD_FILTER(
src
    hb_sha1.h
    hb_sha1.c
    
    hb_rand.h
    hb_rand.c
    
    hb_base16.h
    hb_base16.c
    
    hb_base64.h
    hb_base64.c
    
    hb_getenv.h
    hb_getenv.c
    
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
    
    hb_strncpyn.h
    hb_strncpyn.c
    
    hb_time.h
    ${hb_time_platform}
    
    hb_clock.h
    hb_clock.c

    hb_date.h
    hb_date.c

    hb_sleep.h
    ${hb_sleep_platform}
    
    hb_uid.h
    hb_uid.c
    
    hb_murmurhash64.h
    hb_murmurhash64.c
    
    hb_hashtable.h
    hb_hashtable.c
    
    hb_file.h
    hb_file.c
    
    hb_array.h
    hb_array.c
    
    hb_vector.h
    hb_vector.c
    
    hb_vectorptr.h
    hb_vectorptr.c
    
    hb_list.h
    hb_list.c
    
    hb_math.h
    hb_math.c
)

HUMMINGBIRD_ADD_LIBRARY()

TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_memory)