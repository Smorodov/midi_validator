cmake_minimum_required(VERSION 3.16)
project(midi_test_generator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

add_executable(test_generator generate.cpp)

target_compile_options(test_generator PRIVATE -Wall -Wextra)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_libraries(test_generator stdc++fs)
endif()