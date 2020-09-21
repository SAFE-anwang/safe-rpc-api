cmake_minimum_required(VERSION 3.0)

FIND_PACKAGE(SQLITE3 REQUIRED)

# Set compiler settings
SET(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall")

# Find test source files
FILE(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/safenode.cpp)
FILE(GLOB ETH_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/safe2eth.cpp 
		           ${CMAKE_CURRENT_SOURCE_DIR}/mysqlite.cpp 
				   ${CMAKE_CURRENT_SOURCE_DIR}/mysqlite.h)
# Include header directory
INCLUDE_DIRECTORIES(/usr/local/include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/..)

#link_directories(${CMAKE_CURRENT_SOURCE_DIR})
LINK_DIRECTORIES(/usr/local/lib)

# Link to the appropriate libraries
LINK_LIBRARIES(safeapi sqlite3 pthread)

# Create new executable
#ADD_EXECUTABLE(safenode ${SRCS})
ADD_EXECUTABLE(safe2eth ${ETH_SRCS} )

# Set different name for executable
#SET_TARGET_PROPERTIES(safenode PROPERTIES OUTPUT_NAME safenode)
SET_TARGET_PROPERTIES(safe2eth PROPERTIES OUTPUT_NAME safe2eth)