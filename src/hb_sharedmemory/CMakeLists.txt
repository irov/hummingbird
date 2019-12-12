PROJECT(hb_sharedmemory C)

IF(WIN32)
    SET(hb_sharedmemory_platform hb_sharedmemory_win32.c)
ELSEIF(UNIX AND NOT ANDROID)
    SET(hb_sharedmemory_platform hb_sharedmemory_unix.c)
ENDIF()

ADD_FILTER(
src
	hb_sharedmemory.h
    ${hb_sharedmemory_platform}
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${HUMMINGBIRD_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_config)