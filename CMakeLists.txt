cmake_minimum_required(VERSION 3.0)

# project
project(c--)

# cpp standard
set(CMAKE_CXX_STANDARD 11)

# headers
include_directories(include)

# source
file(GLOB SOURCES "src/*.cpp")

# compiler flag
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-g -Wall")

# traget
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
add_executable(c-- ${SOURCES})

# install
install(FILES c-- DESTINATION /usr/bin/c--)
