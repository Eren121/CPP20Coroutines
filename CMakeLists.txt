cmake_minimum_required(VERSION 3.19)
project(CPP20Coroutines)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS -fcoroutines)
add_executable(CPP20Coroutines src/task_void.cpp include/task.hpp src/catch.cpp src/debug.hpp include/suspend_maybe.hpp src/task_nonvoid.cpp include/generator.hpp)

target_compile_options(CPP20Coroutines PRIVATE -W -Wall -Wextra -pedantic)

## Boost

Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v2.13.6)
FetchContent_MakeAvailable(Catch2)
target_link_libraries(CPP20Coroutines PRIVATE Catch2::Catch2)

include_directories(include src)