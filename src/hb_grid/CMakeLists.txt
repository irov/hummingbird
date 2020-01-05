PROJECT(hb_grid C)

ADD_FILTER(
src
	hb_grid.h
	hb_grid.c
)

ADD_FILTER(
request
    hb_grid_request_api.c
    hb_grid_request_command.c
    hb_grid_request_upload.c
    hb_grid_request_loginuser.c
    hb_grid_request_newuser.c
    hb_grid_request_newproject.c
    hb_grid_request_newaccount.c
    hb_grid_request_loginaccount.c
)

ADD_FILTER(
process
    hb_grid_process_newaccount.h
    hb_grid_process_newaccount.c
    hb_grid_process_loginaccount.h
    hb_grid_process_loginaccount.c
    hb_grid_process_newuser.h
    hb_grid_process_newuser.c
    hb_grid_process_loginuser.h
    hb_grid_process_loginuser.c
    hb_grid_process_newproject.h
    hb_grid_process_newproject.c
    hb_grid_process_upload.h
    hb_grid_process_upload.c
    hb_grid_process_script_api.h
    hb_grid_process_script_api.c
    hb_grid_process_script_event.h
    hb_grid_process_script_event.c
    hb_grid_process_script_command.h
    hb_grid_process_script_command.c
)

INCLUDE_DIRECTORIES(${HUMMINGBIRD_DEPENDENCIES_LIBRARY_DIRECTORY}/libevent/install)
INCLUDE_DIRECTORIES(${HUMMINGBIRD_DEPENDENCIES_SOURCE_DIRECTORY}/libevent/include)

IF(WIN32)
    ADD_EXECUTABLE(${PROJECT_NAME} WIN32 ${HUMMINGBIRD_SOURCE_FILES})
ELSEIF(UNIX AND NOT ANDROID)
    ADD_EXECUTABLE(${PROJECT_NAME} ${HUMMINGBIRD_SOURCE_FILES})
ENDIF()

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${HUMMINGBIRD_DEPENDENCIES_LIBRARY_DIRECTORY}/${CMAKE_STATIC_LIBRARY_PREFIX}event${CMAKE_STATIC_LIBRARY_SUFFIX})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_config)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_log)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_log_tcp)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_log_file)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_script)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_storage)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_json)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_token)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_thread)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_cache)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_db)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_http)
