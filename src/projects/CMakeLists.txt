cmake_minimum_required(VERSION 3.0)

# Set compiler settings
SET(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall")

# Find required packages
FIND_PACKAGE(Boost REQUIRED COMPONENTS system filesystem unit_test_framework)

# Find test source files
FILE(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/safenode.cpp)

# Include header directory
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/..)

#link_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Link to the appropriate libraries
LINK_LIBRARIES(bitcoinapi boost_system boost_filesystem boost_unit_test_framework)

# Create new executable
ADD_EXECUTABLE(safenode ${SRCS})

# Set different name for executable
SET_TARGET_PROPERTIES(safenode PROPERTIES OUTPUT_NAME safenode)
