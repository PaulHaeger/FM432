cmake_minimum_required(VERSION 3.23)
set(NAME "fm432")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchain/msp432toolchain.cmake")
project(FM432)

set(SRCS "${CMAKE_SOURCE_DIR}/src/audio_output.cpp"
    "${CMAKE_SOURCE_DIR}/src/FMOscillator.cpp"
    "${CMAKE_SOURCE_DIR}/src/FMSynth.cpp"
    "${CMAKE_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/MidiParser.cpp"
    "${CMAKE_SOURCE_DIR}/src/MidiTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/OSCParam.cpp"
    )

set(CMAKE_CXX_STANDARD 17)
set(CCS_ROOT ~/ti/ccs1200/ccs)
set(DSLITE  ${CCS_ROOT}/ccs_base/DebugServer/bin/DSLite CACHE FILEPATH "Path to th e DSLite executable for flashing.")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math -O3")

add_compile_definitions(__MSP432P401R__)
add_executable(${PROJECT_NAME} ${SRCS} ${SRC_TOOLCHAIN})

set(YAHAL_DIR ${CMAKE_SOURCE_DIR}/YAHAL)
link_directories(${YAHAL_DIR}/)
include_directories(${YAHAL_DIR}/include/interface)
include_directories(${YAHAL_DIR}/include/platform/msp432)
include_directories(${YAHAL_DIR}/include/platform/msp432/CMSIS)
include_directories(${YAHAL_DIR}/src/platform/msp432)
include_directories(${YAHAL_DIR}/src/util)
include_directories(${YAHAL_DIR}/src/driver)
include_directories(${YAHAL_DIR}/src/task)
target_link_libraries(FM432 nosys m stdc++ c_nano gcc ${YAHAL_DIR}/libYAHAL_msp432.a)

add_custom_target(flash COMMAND ${DSLITE} load -c ${CMAKE_SOURCE_DIR}/toolchain/msp432/MSP432P401R.ccxml -f ${PROJECT_NAME}
                        DEPENDS ${PROJECT_NAME})
