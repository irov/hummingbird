PROJECT(hb_events C)

ADD_FILTER(
src
	hb_events.h
	hb_events.c
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${HUMMINGBIRD_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_db)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_cache)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_config)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_utils)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_log)