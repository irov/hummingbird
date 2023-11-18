PROJECT(hb_thread LANGUAGES C)

IF(HUMMINGBIRD_TARGET_WIN32)
    SET(hb_thread_platform hb_thread_win32.c)
ELSEIF(HUMMINGBIRD_TARGET_UNIX OR HUMMINGBIRD_TARGET_OSX)
    SET(hb_thread_platform hb_thread_pthread.c)
ENDIF()

HUMMINGBIRD_ADD_FILTER(
src
    hb_thread.h
    ${hb_thread_platform}
)

HUMMINGBIRD_ADD_LIBRARY()

TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_memory)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_log)