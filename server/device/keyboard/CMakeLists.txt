FILE(GLOB_RECURSE SRCS ./*.cpp)
add_library(keyboard ${SRCS})
target_link_libraries(keyboard c++ lkernel lalr)