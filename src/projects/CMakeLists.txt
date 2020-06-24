cmake_minimum_required(VERSION 3.0)

# Set compiler settings
SET(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall")

# Find test source files
FILE(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/safenode.cpp)

# Include header directory
INCLUDE_DIRECTORIES(/usr/local/include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/..)

#link_directories(${CMAKE_CURRENT_SOURCE_DIR})
LINK_DIRECTORIES(/usr/local/lib)

# Link to the appropriate libraries
LINK_LIBRARIES(safeapi)

# Create new executable
ADD_EXECUTABLE(safenode ${SRCS})

# Set different name for executable
SET_TARGET_PROPERTIES(safenode PROPERTIES OUTPUT_NAME safenode)
