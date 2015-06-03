
add_subdirectory(x86)
add_subdirectory(memory)
add_subdirectory(thread)
add_subdirectory(cpu)
add_subdirectory(syscall)

SET(MAIN_SRCS Main.cpp Multiboot.cpp)
add_executable(kernel ${MAIN_SRCS})

SET(KERNEL_LIB cpu thread memory arch)
SET(SUPPORT_LIB c lalr exec c++)
target_link_libraries(kernel ${KERNEL_LIB} ${SUPPORT_LIB})