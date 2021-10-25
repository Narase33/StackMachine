# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)

project ("FuncStack")

add_executable(FuncStack FuncStack/FuncStack.cpp)

target_compile_options(FuncStack PUBLIC "/permissive-")

target_include_directories(FuncStack PUBLIC
	${CMAKE_SOURCE_DIR}/src
)