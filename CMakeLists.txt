cmake_minimum_required(VERSION 3.0.0)
project(emulator_network VERSION 0.3.0)

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

set(SOURCE_FILES
    src/Client.cpp
    src/Dispatcher.cpp
    src/Scheduler.cpp
    src/Service.cpp
    src/Session.cpp
)

set (MAIN_SOURCE "")
if (NOT RUN_SERVER)
    add_library(
        emulator_network
        ${SOURCE_FILES}
    )
else()
    add_executable(
        emulator_network
        src/Server.cpp
        ${SOURCE_FILES}
    )
endif()

target_link_libraries(emulator_network ${CONAN_LIBS} ${EXTRA_LIBRARIES_PER_PLATFORM})
