add_subdirectory(tty)

aux_source_directory(./ SRCS)
add_library(drivers ${SRCS})
target_link_libraries(drivers tty)