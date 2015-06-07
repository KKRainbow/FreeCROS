aux_source_directory(./ SRCS)

add_library(misc ${SRCS})

target_link_libraries(misc memory arch)