cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(bfs_integer_vector LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)

set(_cann_hints "$ENV{ASCEND_HOME_PATH}" "$ENV{ASCEND_CANN_PACKAGE_PATH}"
    "/home/developer/Ascend/cann-9.0.0" "/usr/local/Ascend/ascend-toolkit/latest")
find_path(BFS_ACL_INCLUDE_DIR acl/acl.h
    HINTS ${_cann_hints} PATH_SUFFIXES include aarch64-linux/include x86_64-linux/include)
find_library(BFS_ASCENDCL_LIBRARY ascendcl
    HINTS ${_cann_hints} PATH_SUFFIXES lib64 aarch64-linux/lib64 x86_64-linux/lib64)
if(NOT BFS_ACL_INCLUDE_DIR OR NOT BFS_ASCENDCL_LIBRARY)
    message(FATAL_ERROR "CANN ACL headers/library missing. Source your CANN set_env.sh first.")
endif()

add_executable(bfs_benchmark_integer_vector bfs_benchmark_integer_vector.asc)
set_target_properties(bfs_benchmark_integer_vector PROPERTIES LINKER_LANGUAGE ASC)
target_compile_options(bfs_benchmark_integer_vector PRIVATE
    "$<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>"
    "$<$<COMPILE_LANGUAGE:ASC>:-O2>")
target_include_directories(bfs_benchmark_integer_vector PRIVATE ${BFS_ACL_INCLUDE_DIR})
target_link_libraries(bfs_benchmark_integer_vector PRIVATE
    ${BFS_ASCENDCL_LIBRARY} m Threads::Threads)
