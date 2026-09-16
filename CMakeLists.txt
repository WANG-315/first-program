cmake_minimum_required(VERSION 4.3)
project(system1 C)

set(CMAKE_C_STANDARD 11)

# 加入头文件的目录
# PROJECT_SOURCE_DIR CMakeLists所在的目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 搜索src目录下的所有文件，赋值给变量SRC_LIST
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LIST)

# 生成可执行文件，第一个参数代表可执行文件名，剩余就是使用的源文件
add_executable(system1 ${SRC_LIST} main.c)


