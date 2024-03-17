cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(. SAX_SRC)

add_library(SAX SHARED ${SAX_SRC})