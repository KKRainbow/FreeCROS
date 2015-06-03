aux_source_directory(./ SRCS)

add_library(memory ${SRCS})

target_link_libraries(memory arch)