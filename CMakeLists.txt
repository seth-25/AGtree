cmake_minimum_required(VERSION 3.23)
project(AGtree)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -mavx -Wall -march=native")

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

include_directories(sax)
add_subdirectory(sax)

include_directories(agtree)
add_subdirectory(agtree)

include_directories(mixtree)
add_subdirectory(mixtree)


add_executable(agtree_range agtree_range.cpp)
target_link_libraries(agtree_range COMMON AGTREE)

add_executable(linear linear.cpp)
target_link_libraries(linear COMMON)

add_executable(mixtree_range mixtree_range.cpp)
target_link_libraries(mixtree_range COMMON MIXTREE SAX)

add_executable(mixtree_knn mixtree_knn.cpp)
target_link_libraries(mixtree_knn COMMON MIXTREE SAX)

add_executable(test test.cpp)
target_link_libraries(test COMMON)