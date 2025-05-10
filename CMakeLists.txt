cmake_minimum_required(VERSION 3.15)
project(ReadoutDataMonitor)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_BUILD_TYPE Debug)

# Enable pedantic compile mode, we want to be as strict as possible
# at compile time rather than find problems at run time which could be too late.
add_compile_options(-Wall -Wextra -pedantic)

add_subdirectory(readout_decoder)
add_subdirectory(networking)

include_directories(src/common src/metric_algs readout_decoder/src networking)

#file(GLOB DECODER_SRC readout_decoder/*.cpp)
file(GLOB NETWORK_SRC networking/tcp*.cpp)
file(GLOB MONITOR_SRC src/common/*.cpp)
file(GLOB METRICS_SRC src/metric_algs/*.cpp)

add_executable(DataMonitor main.cpp
                ${MONITOR_SRC}
                ${METRICS_SRC}
                ${NETWORK_SRC})

target_link_libraries(DataMonitor PRIVATE pthread)
target_link_libraries(DataMonitor PRIVATE raw_decoder)
