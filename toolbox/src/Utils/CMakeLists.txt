project(utils)


# Add some libraries
add_library(CureUtils  src/ThreadWrapper.cc src/MutexHelper.cc src/RoboLookProxy.cc src/KeyReader.cc  src/CureDebug.cc  src/SocketWrapper.cc )




add_executable(testKeyReader src/testKeyReader.cc)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(CureUtils PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE )

target_link_libraries(testKeyReader CureUtils)


# Link the libraries

#target_link_libraries(dtools PUBLIC xtf_data std_data navi_data xtf_reader lat_long_utm ${OpenCV_LIBS})

set(CURE_TOOLSBOX_UTILS_LIBS CureUtils)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS ${CURE_TOOLSBOX_UTILS_LIBS} EXPORT UtilsConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT UtilsConfig DESTINATION share/Utils/cmake)

# This makes the project importable from the build directory
export(TARGETS ${CURE_TOOLSBOX_UTILS_LIBS} FILE UtilsConfig.cmake)
