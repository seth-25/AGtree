cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(. COMMON_SRC)

add_library(COMMON SHARED ${COMMON_SRC})