add_subdirectory(tty)
add_subdirectory(hd)
add_subdirectory(buffer)

aux_source_directory(./ SRCS)
add_library(drivers ${SRCS})
target_link_libraries(drivers tty)