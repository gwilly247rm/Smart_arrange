# This will define the following variables::
#
#   Serialport_FOUND    - True if the system has the libserialport library
#   SERIALPORT_INCLUDE_DIRS     - location of header files
#   SERIALPORT_LIBRARIES        - location of library files

find_package(PkgConfig)
pkg_check_modules(PC_SERIALPORT QUIET libserialport)

find_path(SERIALPORT_INCLUDE_DIRS libserialport.h
    HINTS ${PC_SERIALPORT_INCLUDE_DIRS} ${PC_SERIALPORT_INCLUDEDIR}
    "/usr/include"
    "/usr/local/include"
	)
find_library(SERIALPORT_LIBRARIES NAMES libserialport.a libserialport.so
    HINTS ${PC_SERIALPORT_LIBDIR} ${PC_SERIALPORT_LIBRARY_DIRS}
    "/usr/lib"
    "/usr/local/lib"
	)
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Serialport
	FOUND_VAR Serialport_FOUND
	REQUIRED_VARS SERIALPORT_INCLUDE_DIRS SERIALPORT_LIBRARIES
	)
