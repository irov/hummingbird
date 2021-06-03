CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(hummingbird_xcode LANGUAGES C)

INCLUDE(../base_template.cmake)
INCLUDE(../xcode_template.cmake)
INCLUDE(../hummingbird_template.cmake)
INCLUDE(../macro_template.cmake)

ADD_SUBDIRECTORY(${HUMMINGBIRD_SOURCE_DIRECTORY} ${CMAKE_BINARY_DIR}/src)