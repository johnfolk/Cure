project(Filters)

# Add some libraries
add_library(CureFilters src/Syncronizer.cc src/ArithmaticFilter.cc  src/PoseErrorModel.cc  src/FuseFilter.cc  src/GyroModel.cc src/PanTiltModel.cc src/SimpleOdoModel.cc src/GyroAdjOdometry.cc  src/DeadReckoner.cc  src/PoseCumulator.cc src/ScanLineFilter.cc src/SLAMFilter.cc src/SensorUser.cc src/MeasSequencer.cc src/FilterAddressMaker.cc   )


add_executable(demo src/demo.cc)
add_executable(demo2 src/demo2.cc)
add_executable(demo3 src/demo3.cc)
add_executable(Pluto2Cure src/Pluto2Cure.cc)
add_executable(Converter src/Converter.cc)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(CureFilters PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE )



# Link the libraries

target_link_libraries(CureFilters PUBLIC CureAddressBank CureTransformation CureMath -lpthread)

set(CURE_TOOLBOX_FILTERS_LIBS CureFilters)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../Math/include ${CMAKE_CURRENT_SOURCE_DIR}/../Transformation/include ${CMAKE_CURRENT_SOURCE_DIR}/../Utils/include ${CMAKE_CURRENT_SOURCE_DIR}/../SensorData/include ${CMAKE_CURRENT_SOURCE_DIR}/../Sensory/include ${CMAKE_CURRENT_SOURCE_DIR}/../AddressBank/include)

# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS ${CURE_TOOLBOX_FILTERS_LIBS} EXPORT CureFiltersConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.

install(EXPORT CureFiltersConfig DESTINATION share/include/cmake)


# This makes the project importable from the build directory

export(TARGETS ${CURE_TOOLSBOX_FILTERS_LIBS} FILE CureFiltersConfig.cmake)




target_link_libraries(Pluto2Cure CureFilters CureAddressBank CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils -pthread)
target_link_libraries(demo CureFilters CureAddressBank CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils -lpthread)
target_link_libraries(demo2 CureFilters CureAddressBank CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils -lpthread)
target_link_libraries(demo3 CureFilters CureAddressBank CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils -lpthread)
target_link_libraries(Converter CureFilters CureAddressBank CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils -lpthread)

