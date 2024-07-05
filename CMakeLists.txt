cmake_minimum_required(VERSION 3.12)
project(AlphaProject)

set(CMAKE_CXX_STANDARD 11)

add_executable(AlphaProject main.cpp Function.cpp Function.h LRPred.cpp LRPred.h types.h Stack.cpp Stack.h buffer.h buffer.c)

include_directories(boost_1_80_0)
link_directories(boost_1_80_0/libs)

target_link_libraries(AlphaProject pthread)