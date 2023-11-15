PROJECT(hb_platform C)

IF(HUMMINGBIRD_TARGET_WIN32)
    SET(hb_platform hb_platform_windows.h)
ENDIF()

HUMMINGBIRD_ADD_FILTER(
src
    ${hb_platform}
)

HUMMINGBIRD_ADD_INTERFACE()