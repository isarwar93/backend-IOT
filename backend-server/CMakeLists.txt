cmake_minimum_required(VERSION 3.20)

set(project_name backend-server)

project(${project_name})

set(CMAKE_CXX_STANDARD 17)

# Source files
include_directories(src)
file(GLOB_RECURSE SOURCES
  src/*.cpp
  src/*.hpp
)

add_library(${project_name}-lib ${SOURCES})

# Oatpp dependencies
find_package(oatpp              1.4.0 REQUIRED)
find_package(oatpp-websocket    1.4.0 REQUIRED)

# Pkg-config dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0)
pkg_check_modules(DBUS REQUIRED dbus-1)
pkg_check_modules(BLUETOOTH REQUIRED bluez)

# Apply include dirs and libs to library
target_include_directories(${project_name}-lib PRIVATE
  ${GLIB2_INCLUDE_DIRS}
  ${DBUS_INCLUDE_DIRS}
  ${BLUETOOTH_INCLUDE_DIRS}
)

target_link_libraries(${project_name}-lib
  oatpp::oatpp
  oatpp::oatpp-websocket
  ${GLIB2_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${BLUETOOTH_LIBRARIES}
)

target_compile_options(${project_name}-lib PRIVATE
  ${GLIB2_CFLAGS_OTHER}
)

# Main executable
add_executable(${project_name}-exe
  src/App.cpp
)

target_link_libraries(${project_name}-exe
  ${project_name}-lib
)

# Test executable
add_executable(${project_name}-test
  test/tests.cpp
  test/WSTest.cpp
  test/WSTest.hpp
)

target_link_libraries(${project_name}-test
  ${project_name}-lib
)

add_dependencies(${project_name}-exe ${project_name}-lib)
add_dependencies(${project_name}-test ${project_name}-lib)

set_target_properties(${project_name}-lib ${project_name}-exe ${project_name}-test PROPERTIES
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF
  CXX_STANDARD_REQUIRED ON
  LINKER_LANGUAGE CXX
)

enable_testing()
add_test(tests ${project_name}-test)
