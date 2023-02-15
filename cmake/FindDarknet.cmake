find_path(DARKNET_INCLUDE_DIRS yolo_v2_class.hpp
    "/usr/include"
    "/usr/local/include"
    )
find_library(DARKNET_LIBRARIES libdarknet.so
    "/usr/lib"
    "/usr/local/lib"
    )
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Darknet
    FOUND_VAR Darknet_FOUND
    REQUIRED_VARS DARKNET_INCLUDE_DIRS DARKNET_LIBRARIES
    )

