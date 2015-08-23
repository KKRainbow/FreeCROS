file(GLOB_RECURSE SRCS ./*.cpp)
add_library(server ${SRCS})
target_link_libraries(server c)

