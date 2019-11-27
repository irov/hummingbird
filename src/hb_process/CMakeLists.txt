PROJECT(hb_process C)

IF(WIN32)
    SET(hb_process_platform hb_process_win32.c)
ELSEIF(UNIX)
    SET(hb_process_platform hb_process_unix.c)
ENDIF()

ADD_FILTER(
src
	hb_process.h
	${hb_process_platform}
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${HUMMINGBIRD_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_log)