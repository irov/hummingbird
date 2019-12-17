PROJECT(hb_script C)

ADD_FILTER(
src
	hb_script.h
	hb_script.c
    hb_script_compiler.h
    hb_script_compiler.c
    hb_script_json.h
    hb_script_json.c
    
    hb_script_handle.h
)

ADD_FILTER(
src_project
    hb_script_server_SetProjectPublicData.c
    hb_script_server_GetProjectPublicData.c
    hb_script_server_UpdateProjectPublicData.c
)

ADD_FILTER(
src_user
    hb_script_server_GetCurrentUserPublicData.c
    hb_script_server_SetCurrentUserPublicData.c
    hb_script_server_UpdateCurrentUserPublicData.c
)

ADD_FILTER(
src_user_entity    
    hb_script_server_CreateUserEntity.c
    hb_script_server_SelectUserEntity.c
    hb_script_server_GetUserEntityPublicData.c
    hb_script_server_SetUserEntityPublicData.c
    hb_script_server_UpdateUserEntityPublicData.c
)

ADD_FILTER(
src_project_entity    
    hb_script_server_CreateProjectEntity.c
    hb_script_server_GetProjectEntity.c
    hb_script_server_SelectProjectEntity.c
    hb_script_server_GetProjectEntityPublicData.c
    hb_script_server_SetProjectEntityPublicData.c
    hb_script_server_UpdateProjectEntityPublicData.c
)

INCLUDE_DIRECTORIES(${HUMMINGBIRD_DEPENDENCIES_SOURCE_DIRECTORY}/lua)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${HUMMINGBIRD_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${HUMMINGBIRD_DEPENDENCIES_LIBRARY_DIRECTORY}/${CMAKE_STATIC_LIBRARY_PREFIX}lua${CMAKE_STATIC_LIBRARY_SUFFIX})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} hb_log)

IF(UNIX AND NOT ANDROID)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} m)
endif()