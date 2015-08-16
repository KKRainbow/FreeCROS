file(GLOB_RECURSE SRCS ./*.cpp)
message(STATUS jfdklsajlkfdsjakljl${SRCS})
add_library(hd ${SRCS})
target_link_libraries(hd ${SUPPORT_LIB} lkernel ramdisk buffer)
