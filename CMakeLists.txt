cmake_minimum_required(VERSION 3.19)
project(CPP20Coroutines)

# Do not set CMake variables, you have to set it manually in your own project. This may be compiler-specific until this is widely supported.
# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_FLAGS -fcoroutines)

add_executable(CPP20CoroutinesTest src/task_void.cpp include/task.hpp src/catch.cpp src/debug.hpp include/suspend_maybe.hpp src/task_nonvoid.cpp include/generator.hpp)

## Boost

Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v2.13.6)
FetchContent_MakeAvailable(Catch2)
target_link_libraries(CPP20CoroutinesTest PRIVATE Catch2::Catch2)
target_include_directories(CPP20CoroutinesTest PRIVATE src)


add_library(CPP20Coroutines INTERFACE include/task.hpp include/suspend_maybe.hpp include/generator.hpp)
target_include_directories(CPP20Coroutines INTERFACE include)

target_link_libraries(CPP20CoroutinesTest PRIVATE CPP20Coroutines)
