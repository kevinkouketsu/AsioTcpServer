cmake_minimum_required(VERSION 3.0.0)
project(cpp_socket_server VERSION 0.1.0)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
    cpp_socket_server
    src/Server.cpp
)

target_link_libraries(cpp_socket_server ${CONAN_LIBS})