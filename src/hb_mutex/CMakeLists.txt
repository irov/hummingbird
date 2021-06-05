PROJECT(hb_mutex C)

IF(HUMMINGBIRD_TARGET_WIN32)
    SET(hb_mutex_platform hb_mutex_win32.c)
    set(hb_mutex_platform_extra_lib)
ELSEIF(HUMMINGBIRD_TARGET_UNIX OR HUMMINGBIRD_TARGET_OSX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    
    set(hb_mutex_platform_extra_lib Threads::Threads)
    
    SET(hb_mutex_platform hb_mutex_pthread.c)
ENDIF()

ADD_FILTER(
src
    hb_mutex.h
    ${hb_mutex_platform}
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${HUMMINGBIRD_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_memory hb_log ${hb_mutex_platform_extra_lib})