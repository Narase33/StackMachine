# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)

project ("FuncStack")

# Include sub-projects.
include_directories("FuncStack")

add_executable(FuncStack FuncStack/FuncStack.cpp)

target_compile_options(FuncStack PUBLIC "/permissive-")