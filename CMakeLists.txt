# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# chỉ tồn tại trong Cmake
set(PROJECT_VER "v1.0.0") 
#set(PROJECT_VER_STRING \"${PROJECT_VER}\")
# tạo ra #define PROJECT_VER "v1.0.1" trong code
add_definitions(-DPROJECT_VER=\"${PROJECT_VER}\") 

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(switch_wifi)
