cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)

include(cmake/HunterGate.cmake)

HunterGate(
    URL "https://github.com/kate-lozovaya/hunter/archive/v0.149.08.tar.gz"
    SHA1 "fdc4ce91f54f6d62846669fb7990c8dc57da691e"
)

project(demo)

hunter_add_package(stack_05)
find_package(stack_05)

add_executable(demo ${CMAKE_CURRENT_SOURCE_DIR}/sources/demo.cpp)
target_link_libraries(demo stack_05)

install(TARGETS demo RUNTIME DESTINATION bin)
