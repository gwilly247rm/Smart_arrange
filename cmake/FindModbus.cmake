# reference: https://github.com/AgileManufacturing/Avans/blob/master/cmake/FindModbus.cmake
# - Try to find Libmodbus
# Once done this will define
#  MODBUS_FOUND - System has Libmodbus
#  MODBUS_INCLUDE_DIRS - The Libmodbus include directories
#  MODBUS_LIBRARIES - The libraries needed to use Libmodbus
#  MODBUS_DEFINITIONS - Compiler switches required for using Libmodbus

find_package(PkgConfig)
pkg_check_modules(PC_MODBUS QUIET libmodbus)
set(MODBUS_DEFINITIONS ${PC_LIBMODBUS_CFLAGS_OTHER})

find_path(MODBUS_INCLUDE_DIRS modbus.h
          HINTS ${PC_MODBUS_INCLUDEDIR} ${PC_MODBUS_INCLUDE_DIRS})

find_library(MODBUS_LIBRARIES NAMES modbus libmodbus
             HINTS ${PC_MODBUS_LIBDIR} ${PC_MODBUS_LIBRARY_DIRS} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set MODBUS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Modbus "Could not find libmodbus" MODBUS_LIBRARIES MODBUS_INCLUDE_DIRS)


mark_as_advanced(MODBUS_INCLUDE_DIRS MODBUS_LIBRARIES)
