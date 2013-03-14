cmake_minimum_required(VERSION 2.8)

set(WARNINGS "-Wall -Wextra -Wzero-as-null-pointer-constant")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -std=c++11 ${WARNINGS}")

include_directories("../mstd/include")
include_directories("../include")

add_subdirectory(xxx)
