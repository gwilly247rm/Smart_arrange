cmake_minimum_required(VERSION 3.10)

# Set variables
set(PROJECT_NAME test)
set(PROJECT_LANGUAGE C CXX)
set(PROJECT_DEPS
    Process SPAData
    SenseCamera FindHole FindScissors FindScissors2 WiseComm
    realsense_lib Hiwin Camera
    modbus hrsdk_ctrl kproto
    yaml-cpp pthread
    ${SERIALPORT_LIBRARIES}    # Modbus:Hiwin
    ${OpenCV_LIBS}
    ${realsense2_LIBRARY}
    ${DARKNET_LIBRARIES}
    ${YAML_CPP_LIBRARIES}
    ${MODBUS_LIBRARIES}
)

# Compile setting
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Set project
project(${PROJECT_NAME} ${PROJECT_LANGUAGE})

# Set file list
file(GLOB PROJECT_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    )

# Build test
foreach(TEST_FILE_PATH ${PROJECT_SRCS})
    # Get each file name
    get_filename_component(TEST_FILE_NAME ${TEST_FILE_PATH} NAME_WE)

    # Build executable
    add_executable(${TEST_FILE_NAME} ${TEST_FILE_PATH})
    set_target_properties(${TEST_FILE_NAME} PROPERTIES
        OUTPUT_NAME ${TEST_FILE_NAME}
        )
    target_link_libraries(${TEST_FILE_NAME} ${PROJECT_DEPS})

    # Install
    install(TARGETS ${TEST_FILE_NAME}
        RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
        )
endforeach()

