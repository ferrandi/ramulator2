cmake_minimum_required(VERSION 3.15)

project(
  Ramulator
  VERSION 2.1
  LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

#### Prompt the build type ####
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()
message("Configuring ${CMAKE_PROJECT_NAME} ${CMAKE_PROJECT_Version}...")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DRAMULATOR_DEBUG")
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
###############################

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

#### External libraries ####
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Skip updating the external dependencies after populating them for the first time")

message("Configuring yaml-cpp...")
option(YAML_CPP_BUILD_CONTRIB "Enable yaml-cpp contrib in library" OFF)
option(YAML_CPP_BUILD_TOOLS "Enable parse tools" OFF)
option(YAML_BUILD_SHARED_LIBS "Build yaml-cpp as a shared library" OFF)
FetchContent_Declare(
  yaml-cpp                             
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG        yaml-cpp-0.9.0                        

)
FetchContent_MakeAvailable(yaml-cpp)
include_directories(${yaml-cpp_SOURCE_DIR}/include)
message("Done configuring yaml-cpp.")

message("Configuring fmt...")
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        10.2.1

)
FetchContent_MakeAvailable(fmt)
include_directories(${fmt_SOURCE_DIR}/include)
message("Done configuring fmt.")

option(RAMULATOR_PYTHON_BINDINGS "Build Python nanobind bindings" OFF)
if(RAMULATOR_PYTHON_BINDINGS)
  find_package(Python 3.10 REQUIRED COMPONENTS Interpreter Development.Module)

  message("Configuring nanobind...")
  FetchContent_Declare(
    nanobind
    GIT_REPOSITORY https://github.com/wjakob/nanobind.git
    GIT_TAG        v2.4.0

  )
  FetchContent_MakeAvailable(nanobind)
  message("Done configuring nanobind.")
endif()
##################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(ramulator SHARED)
set_target_properties(ramulator PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)
# PUBLIC so consumers of the ramulator target get the include path transitively
add_library(Ramulator::ramulator ALIAS ramulator)
target_include_directories(ramulator PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(
  ramulator
  PRIVATE fmt::fmt
  PRIVATE yaml-cpp::yaml-cpp
)

find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/legacy_catalog.inc"
  COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/python"
    ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/src/ramulator/bambu/generate_legacy_catalog.py"
    "${CMAKE_CURRENT_BINARY_DIR}/legacy_catalog.inc"
  DEPENDS src/ramulator/bambu/generate_legacy_catalog.py
    python/ramulator/dram/spec.py python/ramulator/dram/ddr3.py
    python/ramulator/dram/ddr4.py python/ramulator/dram/ddr5.py python/ramulator/dram/hbm2.py)
target_sources(ramulator PRIVATE src/ramulator/bambu/legacy_config.cpp "${CMAKE_CURRENT_BINARY_DIR}/legacy_catalog.inc")
target_include_directories(ramulator PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
add_subdirectory(src/ramulator)

if(RAMULATOR_PYTHON_BINDINGS)
  # Run codegen before building — generates C++ headers from DRAM DSL
  # and Python wrappers from C++ source. Only writes changed files.
  # Post-codegen: run clang-format on generated C++ to match repo style.
  find_program(CLANG_FORMAT clang-format)
  add_custom_target(codegen
    COMMAND ${CMAKE_COMMAND} -E env
      "PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/python"
      ${Python_EXECUTABLE} -m ramulator codegen
      --src-dir ${CMAKE_CURRENT_SOURCE_DIR}/src/ramulator
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running ramulator codegen..."
  )
  if(CLANG_FORMAT)
    add_custom_target(codegen_format
      COMMAND find ${CMAKE_CURRENT_SOURCE_DIR}/src/ramulator/dram/impl -name "*.cpp" -exec ${CLANG_FORMAT} -i {} +
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Formatting generated C++ files..."
    )
    add_dependencies(codegen_format codegen)
    add_dependencies(ramulator codegen_format)
  else()
    message(STATUS "clang-format not found — generated C++ files will not be auto-formatted")
    add_dependencies(ramulator codegen)
  endif()

  nanobind_add_module(_ramulator src/ramulator/python/bindings.cpp)
  target_link_libraries(_ramulator PRIVATE ramulator)
  target_include_directories(_ramulator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
  set_target_properties(_ramulator PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python/ramulator
  )

  nanobind_add_module(_ramulator_test tests/utils/test_harness.cpp)
  target_link_libraries(_ramulator_test PRIVATE ramulator)
  target_include_directories(_ramulator_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
  set_target_properties(_ramulator_test PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python/ramulator
  )
endif()
# Bambu's C++ co-simulation needs no Python runtime or generated source mutation.
add_executable(ramulator-exe src/bambu_main.cpp)
target_link_libraries(ramulator-exe PRIVATE ramulator)
set_target_properties(ramulator-exe PROPERTIES OUTPUT_NAME ramulator2
  INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
install(TARGETS ramulator ramulator-exe EXPORT RamulatorTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY src/ramulator DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
# yaml-cpp is also used by MDPI to resolve includes before passing native YAML.
install(TARGETS yaml-cpp ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY "${yaml-cpp_SOURCE_DIR}/include/yaml-cpp" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
set(_ramulator_cmake_dir "${CMAKE_INSTALL_LIBDIR}/cmake/Ramulator")
configure_package_config_file(cmake/RamulatorConfig.cmake.in
  "${PROJECT_BINARY_DIR}/RamulatorConfig.cmake" INSTALL_DESTINATION "${_ramulator_cmake_dir}")
write_basic_package_version_file("${PROJECT_BINARY_DIR}/RamulatorConfigVersion.cmake"
  VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(EXPORT RamulatorTargets NAMESPACE Ramulator:: DESTINATION "${_ramulator_cmake_dir}")
install(FILES "${PROJECT_BINARY_DIR}/RamulatorConfig.cmake"
  "${PROJECT_BINARY_DIR}/RamulatorConfigVersion.cmake" DESTINATION "${_ramulator_cmake_dir}")
install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/ramulator)
install(FILES "${yaml-cpp_SOURCE_DIR}/LICENSE" DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/ramulator/yaml-cpp)
install(FILES "${fmt_SOURCE_DIR}/LICENSE" DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/ramulator/fmt)
