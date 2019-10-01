project(AddressBank)

# Add some libraries
add_library(CureAddressBank src/CommandAddress.cc src/CommandExecuter.cc src/DataPort.cc src/FilterBankAddress.cc src/SmartDataList.cc src/ServiceAddress.cc src/AddressBank.cc src/Address.cc src/AddressList.cc src/FileAddress.cc src/DataFilter.cc  src/DataFilterAddress.cc src/DataSlot.cc src/DataSlotAddress.cc src/ThreadMerger.cc src/ThreadSwitcher.cc src/PoseFilter.cc  src/DebugAddress.cc src/AddressFcnHook.cc src/ConfigFileReader.cc src/FileAddressManager.cc src/AddressMaker.cc src/RLDisplayHelper.cc src/RLDisplaySICK.cc src/PoseArchive.cc src/DeviceAddress.cc src/ServiceAddressProxy.cc src/SocketAddress.cc)

add_executable(testSmartDataList src/testSmartDataList.cc)
add_executable(testa src/testa.cc)
add_executable(testConfigFileReader src/testConfigFileReader.cc)
add_executable(ConvertSICK2ver4 src/ConvertSICK2ver4.cc)
add_executable(testDataSlotAddress src/testDataSlotAddress.cc)
add_executable(testFileAddressManager src/testFileAddressManager.cc)
add_executable(testAddressFcnHook src/testAddressFcnHook.cc)
add_executable(testPoseArchive src/testPoseArchive.cc)
add_executable(setMeasurementInfo src/setMeasurementInfo.cc)
add_executable(testPackedData  src/testPackedData)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(CureAddressBank PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE )



# Link the libraries

target_link_libraries(CureAddressBank PUBLIC CureTransformation CureMath -lpthread)

set(CURE_TOOLBOX_ADDRESSBANK_LIBS CureAddressBank)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../Math/include ${CMAKE_CURRENT_SOURCE_DIR}/../Transformation/include ${CMAKE_CURRENT_SOURCE_DIR}/../Utils/include ${CMAKE_CURRENT_SOURCE_DIR}/../SensorData/include ${CMAKE_CURRENT_SOURCE_DIR}/../Sensory/include)

# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS ${CURE_TOOLBOX_ADDRESSBANK_LIBS} EXPORT CureAddressBankConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.

install(EXPORT CureAddressBankConfig DESTINATION share/include/cmake)


# This makes the project importable from the build directory

export(TARGETS ${CURE_TOOLSBOX_ADDRESSBANK_LIBS} FILE CureAddressBankConfig.cmake)



target_link_libraries(testa CureAddressBank CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils -lpthread )

target_link_libraries(testSmartDataList CureAddressBank CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils)
target_link_libraries(testConfigFileReader CureAddressBank CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils)
target_link_libraries(ConvertSICK2ver4 CureAddressBank CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils -lpthread)
target_link_libraries(testDataSlotAddress CureAddressBank CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils)
target_link_libraries(testFileAddressManager CureAddressBank CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils)
target_link_libraries(testAddressFcnHook CureAddressBank CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils -lpthread)
target_link_libraries(testPoseArchive CureAddressBank CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils -lpthread)
target_link_libraries(setMeasurementInfo CureAddressBank CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils)
target_link_libraries(testPackedData  CureAddressBank CureSensory CureSensorData CureMath CureTransformation CureGeometry CureUtils)
