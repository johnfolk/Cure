cmake_minimum_required(VERSION 2.8)
project(curetoolbox)

#add_definitions(-std=c++11 -O3)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -O3 -fPIC")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)
option(CURE_EXPORT_BUILD "Export build dir configs"              ON)
add_subdirectory(cxxopts)


add_subdirectory(Utils)
add_subdirectory(Math)
add_subdirectory(Transformation)
add_subdirectory(Geometry)
add_subdirectory(SensorData)
add_subdirectory(Sensory)
add_subdirectory(AddressBank)
add_subdirectory(Filters)

