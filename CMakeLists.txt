cmake_minimum_required(VERSION 3.0.0)
project(cpp_socket_server VERSION 0.1.0)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

set(EXTRA_LIBRARIES_PER_PLATFORM "")
if (MSVC)
    add_definitions(-D_WIN32_WINNT=0x0501)
elseif (UNIX)
    set(EXTRA_LIBRARIES_PER_PLATFORM ${EXTRA_LIBRARIES_PER_PLATFORM} -lpthread)
endif()

add_executable(
    cpp_socket_server
    src/Dispatcher.cpp
    src/Scheduler.cpp
    src/Server.cpp
)

target_link_libraries(cpp_socket_server ${CONAN_LIBS} ${EXTRA_LIBRARIES_PER_PLATFORM})