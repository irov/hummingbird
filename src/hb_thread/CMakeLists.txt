PROJECT(hb_thread C)

IF(WIN32)
    SET(hb_thread_platform hb_thread_win32.c)
ELSEIF(UNIX)
    SET(hb_thread_platform hb_thread_unix.c)
ENDIF()

ADD_FILTER(
src
	hb_thread.h
	${hb_thread_platform}
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${HUMMINGBIRD_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_log)


