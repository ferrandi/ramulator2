cmake_minimum_required(VERSION 3.14)

project(
  ExternalSimulator
  VERSION 1.0
  LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find the parent Ramulator2 build
set(RAMULATOR2_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(RAMULATOR2_SRC_DIR ${RAMULATOR2_ROOT_DIR}/src)

# Include directories
include_directories(${RAMULATOR2_SRC_DIR})
include_directories(${RAMULATOR2_ROOT_DIR}/ext/yaml-cpp/include)
# spdlog removed - using simple logging instead

# Find the Ramulator library
find_library(RAMULATOR_LIB
    NAMES ramulator libramulator
    PATHS ${RAMULATOR2_ROOT_DIR}
    NO_DEFAULT_PATH
    REQUIRED
)

if(NOT RAMULATOR_LIB)
    message(FATAL_ERROR "Ramulator library not found. Please build Ramulator2 first by running 'cd .. && cmake -B build && cmake --build build'")
endif()

message(STATUS "Found Ramulator library: ${RAMULATOR_LIB}")

# Create the external simulator executable
add_executable(external_simulator
    main.cpp
    mock_simulator.cpp
    external_frontend.cpp
)

# Find yaml-cpp static library
find_library(YAML_CPP_LIB
    NAMES yaml-cpp libyaml-cpp.a
    PATHS ${RAMULATOR2_ROOT_DIR}/build/_deps/yaml-cpp-build
    NO_DEFAULT_PATH
    REQUIRED
)

if(NOT YAML_CPP_LIB)
    message(FATAL_ERROR "yaml-cpp library not found")
endif()

message(STATUS "Found yaml-cpp library: ${YAML_CPP_LIB}")

# Link libraries
target_link_libraries(external_simulator
    PRIVATE ${RAMULATOR_LIB}
    PRIVATE ${YAML_CPP_LIB}
    PRIVATE pthread
)

# Set output directory
set_target_properties(external_simulator
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler flags
target_compile_options(external_simulator PRIVATE -Wall -Wextra -O2)

# Print build information
message(STATUS "Building External Simulator")
message(STATUS "  - C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  - Ramulator2 Root: ${RAMULATOR2_ROOT_DIR}")
message(STATUS "  - Ramulator Library: ${RAMULATOR_LIB}")
message(STATUS "  - Output: ${CMAKE_CURRENT_SOURCE_DIR}/external_simulator")

# Copy example configuration
configure_file(
    ${RAMULATOR2_ROOT_DIR}/example_config.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/example_config.yaml
    COPYONLY
)