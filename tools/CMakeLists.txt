cmake_minimum_required(VERSION 3.0.0)

set(app_name "genstruct")

add_compile_options(-std=c++11)

include_directories(
    "D:\\src\\protobuf-3.15.1\\src"
)
link_directories(
    "D:\\src\\protobuf-3.15.1\\cmake"   
)

file(GLOB SRCS "./*.cc" "./*.cpp")
add_executable(${app_name} ${SRCS})

target_link_libraries(${app_name} protobuf)