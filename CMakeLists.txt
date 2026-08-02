cmake_minimum_required(VERSION 3.14)

project(eAVR-oss LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include headers
include_directories(
    .
    src
)

# atmega328p emulator
set(EMULATOR_SOURCES
    src/arch/avr/atmega328p.cc
    tools/emulator.cc
)

add_executable(emu_atmega328p ${EMULATOR_SOURCES})

# target include
target_include_directories(emu_atmega328p PRIVATE
    ${CMAKE_SOURCE_DIR}/auvia
)

# tools
target_compile_options(emu_atmega328p PUBLIC
    -include ${CMAKE_SOURCE_DIR}/src/arch/avr/config.h
)

# auria library
add_subdirectory(auvia)
target_compile_options(auvia PUBLIC
    -include ${CMAKE_SOURCE_DIR}/src/arch/avr/config.h
)

# target linking
target_link_libraries(emu_atmega328p PRIVATE auvia)
