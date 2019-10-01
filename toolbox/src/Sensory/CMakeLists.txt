project(Sensory)

# Add some libraries
add_library(CureSensory src/CameraHelper.cc src/RangeHough.cc src/OdoHelper.cc)

add_executable(camCalib src/camCalib.cc)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(CureSensory PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE )



# Link the libraries

#target_link_libraries(CureGEometry PUBLIC CureTransformation CureMath)

set(CURE_TOOLBOX_SENSORY_LIBS CureSensory)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../Math/include ${CMAKE_CURRENT_SOURCE_DIR}/../Transformation/include ${CMAKE_CURRENT_SOURCE_DIR}/../Utils/include ${CMAKE_CURRENT_SOURCE_DIR}/../SensorData/include)

# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS ${CURE_TOOLBOX_SENSORY_LIBS} EXPORT CureSensoryConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.

install(EXPORT CureSensoryConfig DESTINATION share/include/cmake)


# This makes the project importable from the build directory

export(TARGETS ${CURE_TOOLSBOX_SENSORY_LIBS} FILE CureSensoryConfig.cmake)



target_link_libraries(camCalib CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils)

