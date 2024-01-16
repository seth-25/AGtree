cmake_minimum_required(VERSION 3.23)
project(AGtree)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -mavx -Wall -march=native")

set(distFunc L2)  # 这里设置distFunc的值，可以根据需要修改L2, ED
#set(distFunc ED)  # 这里设置distFunc的值，可以根据需要修改L2, ED

#if(${distFunc} STREQUAL "L2")
#    set(AGTREE agtree/AGtree.cpp)
#else()
#
#endif()

add_definitions(-D${distFunc})

include_directories(common)
add_subdirectory(common)

include_directories(agtree)
add_subdirectory(agtree)

add_executable(range range.cpp)
target_link_libraries(range COMMON AGTREE)

add_executable(linear linear.cpp)
target_link_libraries(linear COMMON)

add_executable(test test.cpp)
target_link_libraries(test COMMON)