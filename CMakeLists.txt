cmake_minimum_required(VERSION 3.29)
project(MinoMalloc C)

set(CMAKE_C_STANDARD 99)

include_directories(includes sources)

if (CMAKE_SYSTEM_NAME MATCHES "Windows" OR
        CMAKE_SYSTEM_NAME MATCHES "MSYS" OR
        CMAKE_SYSTEM_NAME MATCHES "CYGWIN")
    add_definitions(-DPlatform_Windows)
    include_directories(includes/windows)
    file(GLOB PLATFORM_SOURCES "sources/windows/*.c" "sources/windows/*.cpp")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    include_directories(includes/macos includes/posix)
    file(GLOB PLATFORM_SOURCES "sources/macos/[^\\.]*.c" "sources/macos/[^\\.]*.m" "sources/posix/[^\\.]*.c")
    add_definitions(-DPlatform_MacOS -DPlatform_POSIX)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    include_directories(includes/linux includes/posix)
    file(GLOB PLATFORM_SOURCES "sources/linux/*.c" "sources/linux/*.cpp" "sources/posix/*.c")
    add_definitions(-DPlatform_Linux -DPlatform_POSIX)
else ()
    message(FATAL_ERROR "Unsupported platform")
endif ()


file(GLOB SOURCES "sources/[^\\.]*.c")

message(STATUS "Files to compile: ${PLATFORM_SOURCES} ${SOURCES}")
add_executable(MinoMalloc ${SOURCES} ${PLATFORM_SOURCES})
