# Set compiler settings
SET(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall")

# Find required packages
FIND_PACKAGE(Boost REQUIRED COMPONENTS system filesystem unit_test_framework)

# Find test source files
FILE(GLOB safeapi_tests_source ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Include header directory
INCLUDE_DIRECTORIES(/usr/local/include ${CMAKE_SOURCE_DIR}/src/)
LINK_DIRECTORIES(/usr/local/lib)

# Create new executable
ADD_EXECUTABLE(tests ${safeapi_tests_source})

# Link to the appropriate libraries
TARGET_LINK_LIBRARIES(tests
    safeapi
    boost_system
    boost_filesystem
    boost_unit_test_framework)

# Set different name for executable
SET_TARGET_PROPERTIES(tests PROPERTIES OUTPUT_NAME safeapi_tests)
