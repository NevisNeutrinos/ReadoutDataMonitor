cmake_minimum_required(VERSION 3.15)
project(ReadoutDataMonitor)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_BUILD_TYPE Debug)

# Enable pedantic compile mode, we want to be as strict as possible
# at compile time rather than find problems at run time which could be too late.
add_compile_options(-Wall -Wextra -pedantic)

add_subdirectory(readout_decoder)
add_subdirectory(networking)

include_directories(readout_decoder networking)

file(GLOB DECODER_SRC readout_decoder/*.cpp)
file(GLOB NETWORK_SRC networking/tcp*.cpp)
file(GLOB MONITOR_SRC src/*.cpp)

add_executable(DataMonitor main.cpp
                ${MONITOR_SRC}
                ${DECODER_SRC}
                ${NETWORK_SRC})

add_library(data_monitor SHARED STATIC
            ${MONITOR_SRC}
            ${DECODER_SRC}
            ${NETWORK_SRC})
