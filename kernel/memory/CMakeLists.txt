aux_source_directory(./ SRCS)

add_library(memory ${SRCS})

add_subdirectory(heap)
target_link_libraries(memory arch heap)