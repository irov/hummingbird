PROJECT(hb_config C)

ADD_FILTER(
src
	hb_config.h
)

ADD_LIBRARY(${PROJECT_NAME} INTERFACE)

if(MSVC)
    add_custom_target(${PROJECT_NAME}.headers SOURCES ${HUMMINGBIRD_SOURCE_FILES})
endif()