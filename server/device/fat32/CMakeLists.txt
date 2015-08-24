aux_source_directory(./ SRCS)
add_library(fat32 ${SRCS})
target_link_libraries(fat32 c++ lkernel lalr)