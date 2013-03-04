file(GLOB_RECURSE sources *.cpp)
add_library(moggle_xxx ${sources})
install(TARGETS moggle_xxx DESTINATION lib)
