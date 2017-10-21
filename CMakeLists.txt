cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)

include(cmake/HunterGate.cmake)

HunterGate(
    URL "https://github.com/kate-lozovaya/hunter/archive/v0.19.119.1.tar.gz"
    SHA1 "374b4ffb06c4f34ec82b9c2de093721946e2294a"
)

project(demo)

hunter_add_package(print)
find_package(print)

add_executable(demo ${CMAKE_CURRENT_SOURCE_DIR}/sources/demo.cpp)
target_link_libraries(demo print)

install(TARGETS demo RUNTIME DESTINATION bin)
