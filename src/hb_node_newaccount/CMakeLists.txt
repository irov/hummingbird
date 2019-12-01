PROJECT(hb_node_newaccount C)

ADD_FILTER(
src
	hb_node_newaccount.h
	hb_node_newaccount.c
)

ADD_EXECUTABLE(${PROJECT_NAME} WIN32 ${HUMMINGBIRD_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_node_main)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_token)
