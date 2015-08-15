aux_source_directory(./ SRCS)
add_library(hd ${SRCS})
target_link_libraries(hd ${SUPPORT_LIB} arch)
