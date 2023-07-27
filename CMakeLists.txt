cmake_minimum_required(VERSION 3.0)
project(WebSocketOSC)

# Set C++11 standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost
find_package(Boost REQUIRED COMPONENTS system)

# Add include directories for Boost and oscpack
include_directories(${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/oscpack)

# Add subdirectory for oscpack
add_subdirectory(oscpack)

# Add your source files
add_executable(WebSocketOSC main.cpp)

# Link against Boost libraries and oscpack
target_link_libraries(WebSocketOSC ${Boost_LIBRARIES} oscpack)

