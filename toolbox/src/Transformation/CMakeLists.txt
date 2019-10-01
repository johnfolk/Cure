project(Tranformation)


# Add some libraries
add_library(CureTransformation   src/Timestamp.cc src/DataDescriptor.cc src/Rotation2D.cc src/Rotation3D.cc src/Pose2D.cc src/Pose3D.cc src/Transformation2D.cc src/Transformation3D.cc src/TimestampedData.cc src/PackedData.cc src/PackedPose.cc src/PoseTree.cc)




add_executable(testp src/testp.cc)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(CureTransformation PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE )



# Link the libraries

#target_link_libraries(dtools PUBLIC xtf_data std_data navi_data xtf_reader lat_long_utm ${OpenCV_LIBS})

set(CURE_TOOLBOX_TRANSFORMATION_LIBS CureTransformation)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../Math/include)

# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS ${CURE_TOOLBOX_TRANSFORMATION_LIBS} EXPORT TransformationConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT TransformationConfig DESTINATION share/include/cmake)

# This makes the project importable from the build directory
export(TARGETS ${CURE_TOOLBOX_TRANSFORMATION_LIBS} FILE TransformationConfig.cmake)

target_link_libraries(testp CureMath CureTransformation)

target_link_libraries(CureTransformation PUBLIC CureMath CureUtils)

