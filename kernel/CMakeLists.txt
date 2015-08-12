
add_subdirectory(memory)
add_subdirectory(thread)
add_subdirectory(cpu)
add_subdirectory(syscall)
add_subdirectory(x86)
add_subdirectory(ramdisk)
add_subdirectory(tty)

SET(MAIN_SRCS Main.cpp Multiboot.cpp)
add_executable(kernel ${MAIN_SRCS})

SET(KERNEL_LIB thread memory cpu arch ramdisk misc tty)
SET(SUPPORT_LIB c lalr exec c++ lkernel)
target_link_libraries(kernel ${KERNEL_LIB} ${SUPPORT_LIB})

add_subdirectory(misc)