CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

OPTION(HUMMINGBIRD_DOWNLOADS_SILENT "download silent" OFF)

PROJECT(downloads)

INCLUDE(../hummingbird_template.cmake)
include(../download_template.cmake)

HUMMINGBIRD_GIT_CLONE(mongo https://github.com/mongodb/mongo-c-driver.git "1.15.1")
HUMMINGBIRD_GIT_CLONE(lua https://github.com/lua/lua.git "v5.3.5")
HUMMINGBIRD_GIT_CLONE(postgres https://github.com/postgres/postgres.git "REL_11_5")
HUMMINGBIRD_GIT_CLONE(lz4 https://github.com/lz4/lz4.git "v1.9.2")
HUMMINGBIRD_GIT_CLONE(libevent https://github.com/libevent/libevent.git "release-2.1.11-stable")
