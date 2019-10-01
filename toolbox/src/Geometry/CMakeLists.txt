project(Geometry)

# Add some libraries
add_library(CureGeometry   src/Line2D.cc src/Line3D.cc src/Plane.cc src/Point2D.cc src/Point3D.cc)

add_executable(testg src/testg.cc)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(CureGeometry PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE )



# Link the libraries

#target_link_libraries(CureGEometry PUBLIC CureTransformation CureMath)

set(CURE_TOOLBOX_GEOMETRY_LIBS CureGeometry)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../Math/include ${CMAKE_CURRENT_SOURCE_DIR}/../Transformation/include)

# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS ${CURE_TOOLBOX_GEOMETRY_LIBS} EXPORT CureGeometryConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.

install(EXPORT CureGeometryConfig DESTINATION share/include/cmake)


# This makes the project importable from the build directory

export(TARGETS ${CURE_TOOLSBOX_GEOMETRY_LIBS} FILE CureGeometryConfig.cmake)



target_link_libraries(testg CureMath CureTransformation CureGeometry CureUtils)

