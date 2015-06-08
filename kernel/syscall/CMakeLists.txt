aux_source_directory(./ SRCS)

add_library(syscall ${SRCS})

target_link_libraries(syscall arch memory ramdisk)