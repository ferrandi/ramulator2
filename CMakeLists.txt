cmake_minimum_required(VERSION 3.15)

project(Ramulator VERSION 2.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(FetchContent)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
add_compile_definitions($<$<CONFIG:Debug>:RAMULATOR_DEBUG>)

# Keep downloaded dependencies in the build tree, leaving the source tree clean.
option(FETCHCONTENT_UPDATES_DISCONNECTED "Skip updating populated dependencies" ON)
option(YAML_CPP_BUILD_CONTRIB "Build yaml-cpp contrib" OFF)
option(YAML_CPP_BUILD_TOOLS "Build yaml-cpp tools" OFF)
option(YAML_CPP_BUILD_TESTS "Build yaml-cpp tests" OFF)
option(YAML_BUILD_SHARED_LIBS "Build yaml-cpp as a shared library" OFF)
FetchContent_Declare(yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG yaml-cpp-0.7.0
)
FetchContent_MakeAvailable(yaml-cpp)

FetchContent_Declare(argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse.git
  GIT_TAG v2.9
)
FetchContent_MakeAvailable(argparse)

# The implementation object libraries use these directory-scoped include paths.
include_directories("${PROJECT_SOURCE_DIR}/src" "${yaml-cpp_SOURCE_DIR}/include")

add_library(ramulator SHARED)
add_library(Ramulator::ramulator ALIAS ramulator)
target_include_directories(ramulator PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ramulator>
)
target_compile_features(ramulator PUBLIC cxx_std_20)
target_link_libraries(ramulator PUBLIC yaml-cpp)
set_target_properties(ramulator PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

add_executable(ramulator-exe)
target_link_libraries(ramulator-exe PRIVATE ramulator argparse)
set_target_properties(ramulator-exe PROPERTIES OUTPUT_NAME ramulator2)
file(RELATIVE_PATH _ramulator_install_libdir
  "${CMAKE_INSTALL_FULL_BINDIR}" "${CMAKE_INSTALL_FULL_LIBDIR}")
if(APPLE)
  set_property(TARGET ramulator-exe PROPERTY INSTALL_RPATH "@loader_path/${_ramulator_install_libdir}")
elseif(UNIX)
  set_property(TARGET ramulator-exe PROPERTY INSTALL_RPATH "$ORIGIN/${_ramulator_install_libdir}")
endif()

add_subdirectory(src)

install(TARGETS ramulator ramulator-exe yaml-cpp EXPORT RamulatorTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ramulator
  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
install(DIRECTORY "${yaml-cpp_SOURCE_DIR}/include/yaml-cpp"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES "${yaml-cpp_SOURCE_DIR}/LICENSE"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/ramulator/yaml-cpp)
install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/ramulator)

set(_ramulator_cmake_dir "${CMAKE_INSTALL_LIBDIR}/cmake/Ramulator")
configure_package_config_file(cmake/RamulatorConfig.cmake.in
  "${PROJECT_BINARY_DIR}/RamulatorConfig.cmake"
  INSTALL_DESTINATION "${_ramulator_cmake_dir}")
write_basic_package_version_file("${PROJECT_BINARY_DIR}/RamulatorConfigVersion.cmake"
  VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(EXPORT RamulatorTargets NAMESPACE Ramulator::
  DESTINATION "${_ramulator_cmake_dir}")
install(FILES "${PROJECT_BINARY_DIR}/RamulatorConfig.cmake"
              "${PROJECT_BINARY_DIR}/RamulatorConfigVersion.cmake"
  DESTINATION "${_ramulator_cmake_dir}")
