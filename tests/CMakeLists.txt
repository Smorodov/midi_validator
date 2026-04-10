cmake_minimum_required(VERSION 3.16)
project(midi_validator_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

add_executable(run_tests
    run_tests.cpp
    test_framework.cpp
    midi_generator.cpp
)

target_compile_options(run_tests PRIVATE -Wall -Wextra)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_libraries(run_tests stdc++fs)
endif()