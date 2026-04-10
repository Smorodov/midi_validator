cmake_minimum_required(VERSION 3.16)
project(midi_validator VERSION 4.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

add_executable(midi_validator
    src/main.cpp
    src/midi_parser.cpp
    src/midi_analyzer.cpp
)

target_compile_options(midi_validator PRIVATE -Wall -Wextra)

# Debug flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(midi_validator PRIVATE -g -O0 -DDEBUG)
    target_compile_definitions(midi_validator PRIVATE _GLIBCXX_DEBUG)
endif()

install(TARGETS midi_validator RUNTIME DESTINATION bin)