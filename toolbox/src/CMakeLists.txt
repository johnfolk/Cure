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

#option(CURE_USE_PYTHON3 "Use python3 instead of python2"         OFF)
option(CURE_EXPORT_BUILD "Export build dir configs"              ON)


#find_package(OpenCV REQUIRED core highgui imgproc)
#find_package(OpenGL REQUIRED)

#include(${CMAKE_CURRENT_BINARY_DIR}/embree/embree-config.cmake)



#if(MSVC)
#  if (CURE_USE_PYTHON3)
#    set(PYTHON_EXECUTABLE C:/Python35/python.exe)
#  else()
#    #set(PYTHON_EXECUTABLE C:/Python27/python.exe)
#    set(PYTHON_EXECUTABLE C:/Python27-x64/python.exe)
#  endif()
#else()
#  if (CURELIB_USE_PYTHON3)
#    set(PYTHON_EXECUTABLE /usr/bin/python3.5)
#  else()
#    set(PYTHON_EXECUTABLE /usr/bin/python2.7)
#  endif()
#endif()

#if (MSVC)
#  set(Boost_USE_STATIC_LIBS ON CACHE BOOL "use static libraries from Boost")
#  set(Boost_USE_MULTITHREADED ON)
#  set(Boost_USE_DEBUG_RUNTIME OFF)
#endif()

# For some reason it seems like we need to do this after libigl
#find_package(Boost COMPONENTS system filesystem date_time REQUIRED)

#add_definitions(-DCERES_GFLAGS_NAMESPACE=${GFLAGS_NAMESPACE})

#set(PYBIND11_CPP_STANDARD -std=c++11)
#add_definitions(-DPYBIND11_PYTHON_VERSION=2.7)
#set(PYBIND11_PYTHON_VERSION 2.7)
#set(PYTHON_LIBRARIES /usr/lib/x86_64-linux-gnu/libpython2.7.so)

#add_subdirectory(pybind11)

#message("Pybind11: " ${PYBIND11_INCLUDE_DIR})

add_subdirectory(cxxopts)

#include_directories(${PYBIND11_INCLUDE_DIR} ${EIGEN3_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/cereal/include ###${TinyXML2_INCLUDE_DIRS})
#link_directories(${Boost_LIBRARY_DIRS})
#include_directories(eigen_cereal/include)
# Utils Math Transformation Geometry SensorData  Sensory AddressBank Filters MSpace
#include(cmake/glad.cmake)
add_subdirectory(Utils)
add_subdirectory(Math)
add_subdirectory(Transformation)
add_subdirectory(Geometry)
add_subdirectory(SensorData)
#add_subdirectory(Sensory)
#add_subdirectory(AddressBank)
#add_subdirectory(Filters)
#add_subdirectory(MSpace)
