aux_source_directory(./ SRCS)
add_library(buffer ${SRCS})
target_link_libraries(buffer ${SUPPORT_LIB} arch)
