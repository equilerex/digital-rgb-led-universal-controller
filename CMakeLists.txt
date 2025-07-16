cmake_minimum_required(VERSION 3.16)

# Set the project name
project(esp32_basic_wearable_fastled)

# Include the PlatformIO build system
include($ENV{HOME}/.platformio/packages/framework-arduinoespressif32/tools/cmake/platformio.cmake)

# Define the source directory
set(SRC_DIR src)

# Add the source files
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.h")

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
# Add any additional libraries here if needed
# target_link_libraries(${PROJECT_NAME} <library_name>)

# Include directories
include_directories(${SRC_DIR})

# Set build flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add PlatformIO targets
platformio_add_target(${PROJECT_NAME})
