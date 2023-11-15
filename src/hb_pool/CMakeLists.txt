PROJECT(hb_pool C)

HUMMINGBIRD_ADD_FILTER(
src
    hb_pool.h
    hb_pool.c
)

HUMMINGBIRD_ADD_LIBRARY()

TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_mutex hb_config)