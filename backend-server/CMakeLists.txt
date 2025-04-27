cmake_minimum_required(VERSION 3.20)

set(project_name backend-server) ## rename your project here

project(${project_name})

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

# Automatically find all .cpp and .hpp files inside src/
file(GLOB_RECURSE SOURCES
  src/*.cpp
  src/*.hpp
)

# Create a library target using all found files
add_library(${project_name}-lib ${SOURCES})

# add_library(${project_name}-lib
#         src/AppComponent.hpp
#         src/controller/RoomsController.hpp
#         src/rooms/GraphListener.hpp
#         src/rooms/GraphListener.cpp
#         src/rooms/Peer.cpp
#         src/rooms/Peer.hpp
#         src/rooms/Room.cpp
#         src/rooms/Room.hpp
#         src/rooms/Lobby.cpp
#         src/rooms/Lobby.hpp
# )

## link libs

find_package(oatpp              1.4.0 REQUIRED)
find_package(oatpp-websocket    1.4.0 REQUIRED)

target_link_libraries(${project_name}-lib
        PUBLIC oatpp::oatpp
        PUBLIC oatpp::oatpp-test
        PUBLIC oatpp::oatpp-websocket
)

## add executables

add_executable(${project_name}-exe
        src/App.cpp
)
target_link_libraries(${project_name}-exe ${project_name}-lib)
add_dependencies(${project_name}-exe ${project_name}-lib)

add_executable(${project_name}-test
        test/tests.cpp
        test/WSTest.cpp
        test/WSTest.hpp
)
target_link_libraries(${project_name}-test ${project_name}-lib)
add_dependencies(${project_name}-test ${project_name}-lib)

set_target_properties(${project_name}-lib ${project_name}-exe ${project_name}-test PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
        LINKER_LANGUAGE CXX
)

enable_testing()
add_test(tests ${project_name}-test)
