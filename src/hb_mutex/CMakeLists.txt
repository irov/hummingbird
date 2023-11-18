PROJECT(hb_mutex LANGUAGES C)

IF(HUMMINGBIRD_TARGET_WIN32)
    SET(hb_mutex_platform hb_mutex_win32.c)
ELSEIF(HUMMINGBIRD_TARGET_UNIX OR HUMMINGBIRD_TARGET_OSX)
    SET(hb_mutex_platform hb_mutex_pthread.c)
ENDIF()

HUMMINGBIRD_ADD_FILTER(
src
    hb_mutex.h
    ${hb_mutex_platform}
)

HUMMINGBIRD_ADD_LIBRARY()

TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_memory)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_log)