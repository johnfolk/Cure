#
#
#
## Makefile.am for the CureFilters library
dummyvar=
# _LTLIBRARIES defines the libraries to make (using libtool)
# cure_libdir defines where to install the libraries
cure_libdir = $(libdir)/cure
cure_lib_LTLIBRARIES = libCureFilters.la
libCureFilters_la_SOURCES =	Syncronizer.cc Syncronizer.hh \
                        ArithmaticFilter.cc ArithmaticFilter.hh \
                         PoseErrorModel.cc PoseErrorModel.hh \
                        FuseFilter.cc FuseFilter.hh \
                        GyroModel.cc GyroModel.hh \
                        PanTiltModel.cc PanTiltModel.hh \
                        SimpleOdoModel.cc SimpleOdoModel.hh \
                        GyroAdjOdometry.cc GyroAdjOdometry.hh \
                        DeadReckoner.cc DeadReckoner.hh \
                        PoseCumulator.cc PoseCumulator.hh \
			ScanLineFilter.cc ScanLineFilter.hh \
			SLAMFilter.cc SLAMFilter.hh \
			SensorUser.cc SensorUser.hh \
			MeasSequencer.cc MeasSequencer.hh \
			FilterAddressMaker.cc FilterAddressMaker.hh 


# _HEADERS defines the headers to export/install
# cure_includedir defines where to install the headers

cure_includedir = $(includedir)/cure/Filters
cure_include_HEADERS =  MotionCommand.hh Syncronizer.hh  \
			ArithmaticFilter.hh PoseErrorModel.hh \
			FuseFilter.hh GyroModel.hh PanTiltModel.hh \
                        SimpleOdoModel.hh PoseCumulator.hh SensorUser.hh\
			 MeasSequencer.hh ScanLineFilter.hh \
                        SLAMFilter.hh DeadReckoner.hh GyroAdjOdometry.hh  \
			FilterAddressMaker.hh 

# AM_CXXFLAGS defines the C++ flags used by automake in this makefile
AM_CXXFLAGS = -I@top_srcdir@/src	

# _PROGRAMS defines the programs/binaries to make
# check defines that this is only made during a 'make check' buil
check_PROGRAMS = Converter Pluto2Cure demo demo2 demo3 


Pluto2Cure_SOURCES = Pluto2Cure.cc
Pluto2Cure_LDADD = libCureFilters.la \
              @top_builddir@/src/Math/libCureMath.la \
              @top_builddir@/src/Transformation/libCureTransformation.la \
	      @top_builddir@/src/AddressBank/libCureAddressBank.la \
	      @top_builddir@/src/Utils/libCureUtils.la \
	      @top_builddir@/src/Sensory/libCureSensory.la \
              @top_builddir@/src/SensorData/libCureSensorData.la -lpthread

Converter_SOURCES = Converter.cc
Converter_LDADD = libCureFilters.la \
              @top_builddir@/src/Math/libCureMath.la \
              @top_builddir@/src/Transformation/libCureTransformation.la \
	      @top_builddir@/src/AddressBank/libCureAddressBank.la \
	      @top_builddir@/src/Utils/libCureUtils.la \
	      @top_builddir@/src/Sensory/libCureSensory.la \
              @top_builddir@/src/SensorData/libCureSensorData.la -lpthread

demo_SOURCES = demo.cc
demo_LDADD = libCureFilters.la \
              @top_builddir@/src/Math/libCureMath.la \
              @top_builddir@/src/Transformation/libCureTransformation.la \
	      @top_builddir@/src/AddressBank/libCureAddressBank.la \
	      @top_builddir@/src/Utils/libCureUtils.la \
	      @top_builddir@/src/Sensory/libCureSensory.la \
              @top_builddir@/src/SensorData/libCureSensorData.la -lpthread

demo2_SOURCES = demo2.cc
demo2_LDADD = libCureFilters.la \
              @top_builddir@/src/Math/libCureMath.la \
              @top_builddir@/src/Transformation/libCureTransformation.la \
	      @top_builddir@/src/AddressBank/libCureAddressBank.la \
	      @top_builddir@/src/Utils/libCureUtils.la \
	      @top_builddir@/src/Sensory/libCureSensory.la \
              @top_builddir@/src/SensorData/libCureSensorData.la -lpthread

demo3_SOURCES = demo3.cc
demo3_LDADD = libCureFilters.la \
              @top_builddir@/src/Math/libCureMath.la \
              @top_builddir@/src/Transformation/libCureTransformation.la \
	      @top_builddir@/src/AddressBank/libCureAddressBank.la \
	      @top_builddir@/src/Utils/libCureUtils.la \
	      @top_builddir@/src/Sensory/libCureSensory.la \
              @top_builddir@/src/SensorData/libCureSensorData.la -lpthread

