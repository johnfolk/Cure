## Makefile.am for the Geometry library
dummyvar=

# _LTLIBRARIES defines the libraries to make (using libtool)
# cure_libdir defines where to install the libraries
cure_libdir = $(libdir)/cure
cure_lib_LTLIBRARIES = libCureGeometry.la
libCureGeometry_la_SOURCES = Line2D.cc Line2D.hh \
                         Line3D.cc Line3D.hh \
                         Plane.cc Plane.hh \
                         Point2D.cc Point2D.hh \
                         Point3D.cc Point3D.hh

# _HEADERS defines the headers to export/install
# cure_includedir defines where to install the headers
cure_includedir = $(includedir)/cure/Geometry
cure_include_HEADERS = Line2D.hh Line3D.hh Plane.hh Point2D.hh Point3D.hh

# AM_CXXFLAGS defines the C++ flags used by automake in this makefile
AM_CXXFLAGS = -I@top_srcdir@/src 
           
# _PROGRAMS defines the programs/binaries to make
# check defines that this is only made during a 'make check' build
check_PROGRAMS = testp
testp_SOURCES = testp.cc
testp_LDADD = libCureGeometry.la \
              @top_builddir@/src/Math/libCureMath.la \
              @top_builddir@/src/Transformation/libCureTransformation.la
