set(CMAKE_MACOSX_RPATH 1)

FIND_PACKAGE(JSONCPP REQUIRED)
FIND_PACKAGE(JSONRPCCPP REQUIRED)
FIND_PACKAGE(CURL REQUIRED)

INCLUDE_DIRECTORIES(/usr/local/include)
LINK_DIRECTORIES(/usr/local/lib)

# Find header and source files
FILE(GLOB safeapi_header ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
FILE(GLOB safeapi_source ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Set target libraries
ADD_LIBRARY(safeapi SHARED ${safeapi_source})
ADD_LIBRARY(safeapi_static STATIC ${safeapi_source})

# Set output name of static library
SET_TARGET_PROPERTIES(safeapi_static PROPERTIES OUTPUT_NAME safeapi)

# Link the library with json-rpc-cpp libs
TARGET_LINK_LIBRARIES(safeapi
                        ${CURL_LIBRARY}
                        jsoncpp
                        jsonrpccpp-common
                        jsonrpccpp-client)

TARGET_LINK_LIBRARIES(safeapi_static
                        ${CURL_LIBRARY}
                        ${JSONCPP_LIBRARY}
                        jsonrpccpp-common
                        jsonrpccpp-client)

# Set version settings
SET(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})
SET_TARGET_PROPERTIES(safeapi safeapi_static PROPERTIES
    VERSION "${VERSION_STRING}"
    SOVERSION "${SO_VERSION}")

# Install to appropriate destinations
INSTALL(FILES ${safeapi_header} DESTINATION include/bitcoinapi)

INSTALL(TARGETS safeapi safeapi_static
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
