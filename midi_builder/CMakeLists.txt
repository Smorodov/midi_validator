cmake_minimum_required(VERSION 3.16)
project(midi_builder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")


add_executable(demo demo.cpp)
target_compile_options(demo PRIVATE -Wall -Wextra)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_libraries(demo stdc++fs)
endif()


# demo_two_instruments.cpp
add_executable(demo_two_instruments demo_two_instruments.cpp)
target_compile_options(demo_two_instruments PRIVATE -Wall -Wextra)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_libraries(demo_two_instruments stdc++fs)
endif()
