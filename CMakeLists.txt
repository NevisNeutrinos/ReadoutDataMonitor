cmake_minimum_required(VERSION 3.15)
project(ReadoutDataMonitor)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_BUILD_TYPE Debug)

# Enable pedantic compile mode, we want to be as strict as possible
# at compile time rather than find problems at run time which could be too late.
add_compile_options(-Wall -Wextra -pedantic)

# This sets the variable for the Decoder
set(USE_PYTHON FALSE)

add_subdirectory(readout_decoder)
add_subdirectory(networking)
add_subdirectory(PGramsCommCodec)

include_directories(src/common
                    src/monitor_algs
                    readout_decoder/src
                    PGramsCommCodec/include
                    networking)

#file(GLOB DECODER_SRC readout_decoder/*.cpp)
file(GLOB NETWORK_SRC networking/tcp*.cpp)
file(GLOB MONITOR_SRC src/common/*.cpp)
file(GLOB MONITOR_ALGS_SRC src/monitor_algs/*.cpp)
file(GLOB MONITOR_STRUCT_SRC PGramsCommCodec/src*.cpp)

add_executable(DataMonitor main.cpp
                ${MONITOR_SRC}
                ${MONITOR_ALGS_SRC}
                ${NETWORK_SRC})

target_link_libraries(DataMonitor PRIVATE datamon_core)
target_link_libraries(DataMonitor PRIVATE pthread)
target_link_libraries(DataMonitor PRIVATE raw_decoder)

#add_executable(TestDataMonitor test_main.cpp
#        ${MONITOR_SRC}
#        ${MONITOR_ALGS_SRC}
#        ${NETWORK_SRC})
#
#target_link_libraries(TestDataMonitor PRIVATE datamon_core)
#target_link_libraries(TestDataMonitor PRIVATE pthread)
#target_link_libraries(TestDataMonitor PRIVATE raw_decoder)