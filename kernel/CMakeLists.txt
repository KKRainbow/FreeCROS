
add_subdirectory(memory)
add_subdirectory(thread)
add_subdirectory(cpu)
add_subdirectory(syscall)
add_subdirectory(x86)

SET(MAIN_SRCS Main.cpp Multiboot.cpp)
add_executable(kernel ${MAIN_SRCS})

SET(KERNEL_LIB thread memory cpu arch)
SET(SUPPORT_LIB c lalr exec c++)
target_link_libraries(kernel memory ${SUPPORT_LIB})

message(STATUS ${KERNEL_LIB})