project(SensorData)

# Add some libraries
add_library(CureSensorData src/Odometry.cc src/PackedSICK.cc src/PackedSonar.cc src/PackedCommand.cc src/PackedGeneric.cc src/MeasurementSet.cc src/PackedMeasurementSet.cc src/Measurement.cc src/SICKScan.cc src/SonarData.cc src/Command.cc src/GenericData.cc src/VisionData.cc src/PackedVision.cc src/SensorData.cc src/GridData.cc src/StateData.cc src/PackedGrid.cc src/PackedState.cc src/SensorPose.cc src/PackedSensorPose.cc src/SensorOffset.cc src/TimestampedString.cc src/PackedTimestampedString.cc src/SmartData.cc src/DataSet.cc)

add_executable(testd src/testd.cc)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(CureSensorData PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE )



# Link the libraries

#target_link_libraries(CureGEometry PUBLIC CureTransformation CureMath)

set(CURE_TOOLBOX_SENSORDATA_LIBS CureSensorData)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../Math/include ${CMAKE_CURRENT_SOURCE_DIR}/../Transformation/include ${CMAKE_CURRENT_SOURCE_DIR}/../Utils/include)

# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS ${CURE_TOOLBOX_SENSORDATA_LIBS} EXPORT CureSensorDataConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.

install(EXPORT CureSensorDataConfig DESTINATION share/include/cmake)


# This makes the project importable from the build directory

export(TARGETS ${CURE_TOOLSBOX_SENSORDATA_LIBS} FILE CureSensorDataConfig.cmake)



target_link_libraries(testd CureSensorData CureMath CureTransformation CureGeometry CureUtils)

