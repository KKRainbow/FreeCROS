aux_source_directory(./ SRCS)

add_library(ramdisk ${SRCS})

target_link_libraries(ramdisk memory cpu thread lalr arch server)