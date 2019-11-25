CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(hummingbird_unix LANGUAGES C)

INCLUDE(../unix_template.cmake)
INCLUDE(../hummingbird_template.cmake)
INCLUDE(../macro_template.cmake)

ADD_SUBDIRECTORY(${HUMMINGBIRD_SOURCE_DIRECTORY} ${CMAKE_BINARY_DIR}/src)