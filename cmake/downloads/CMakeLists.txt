CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

OPTION(HUMMINGBIRD_DOWNLOADS_SILENT "download silent" OFF)

PROJECT(downloads)

INCLUDE(../base_template.cmake)
INCLUDE(../hummingbird_template.cmake)
include(../download_template.cmake)

HUMMINGBIRD_GIT_CLONE(zlib https://github.com/madler/zlib.git "v1.2.11")
HUMMINGBIRD_GIT_CLONE(mongo https://github.com/mongodb/mongo-c-driver.git "1.17.6")
HUMMINGBIRD_GIT_CLONE(lua https://github.com/lua/lua.git "v5.4.3")
HUMMINGBIRD_GIT_CLONE(lz4 https://github.com/lz4/lz4.git "v1.9.3")
HUMMINGBIRD_GIT_CLONE(libevent https://github.com/libevent/libevent.git "release-2.1.12-stable")
HUMMINGBIRD_GIT_CLONE(jansson https://github.com/akheron/jansson "v2.13.1")
HUMMINGBIRD_GIT_CLONE(hiredis https://github.com/redis/hiredis.git "v1.0.0")
