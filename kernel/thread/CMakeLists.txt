aux_source_directory(./ SRCS)

add_library(thread ${SRCS})
target_link_libraries(thread ${SUPPORT_LIB} arch)