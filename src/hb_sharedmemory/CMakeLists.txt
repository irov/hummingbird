PROJECT(hb_sharedmemory C)

ADD_FILTER(
src
	hb_sharedmemory.h
	hb_sharedmemory.c
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${HUMMINGBIRD_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_config)