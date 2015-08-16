add_subdirectory(buffer)
add_subdirectory(tty)
add_subdirectory(hd)

aux_source_directory(./ SRCS)
add_library(drivers ${SRCS})
target_link_libraries(drivers tty hd)