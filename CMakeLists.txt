cmake_minimum_required(VERSION 2.6)
project(freecros)

SET(ARCHITECT "x86")

SET(CMAKE_CXX_FLAGS "-fomit-frame-pointer -g -O0 -ffreestanding -fno-builtin -fno-rtti -std=c++11 -m32 -fno-exceptions -nostdlib -nodefaultlibs -nostartfiles -nostdinc -nostdinc++ -Wreturn-type")
SET(CMAKE_C_FLAGS "-fomit-frame-pointer -g -O0 -m32 -ffreestanding -fno-builtin -nostdlib -nodefaultlibs -nostartfiles -nostdinc -Wreturn-type")

SET(Link_Script ${PROJECT_SOURCE_DIR}/config/${ARCHITECT}/kernel.ld)
SET(CMAKE_EXE_LINKER_FLAGS "-T${Link_Script}")

ADD_DEFINITIONS("-D__CROS")
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib/liblalr/Lalrlib/library)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib/libc)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib/libexec)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib/libc++)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib/libkernel)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib/libkernel/${ARCHITECT})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/kernel/${ARCHITECT})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/kernel)


SET(ISO_DIR ${PROJECT_BINARY_DIR}/iso)

add_subdirectory(kernel)
add_subdirectory(config)
add_subdirectory(lib)
# add_subdirectory(test)


ADD_CUSTOM_TARGET(
	ISO ALL
	COMMAND mkdir ${ISO_DIR}/boot/grub -p
	COMMAND cp ./kernel/kernel ${ISO_DIR}/boot/myos.bin 
	COMMAND cp ${PROJECT_SOURCE_DIR}/config/${ARCHITECT}/gdbcommand ${PROJECT_BINARY_DIR}/
	COMMAND cp ${PROJECT_SOURCE_DIR}/config/${ARCHITECT}/bochs.bxrc ${PROJECT_BINARY_DIR}/
	COMMAND grub-mkrescue -o ${PROJECT_BINARY_DIR}/myos.iso ${ISO_DIR}
	COMMENT "generating iso file"
	DEPENDS kernel
	)
ADD_CUSTOM_TARGET(
	bochsdbg
	COMMAND bochsgdb -q -f bochs.bxrc
	)
ADD_CUSTOM_TARGET(
	bochs
	COMMAND bochs -q -f bochs.bxrc
	)
ADD_CUSTOM_TARGET(
	run
	COMMAND qemu-system-i386 -cdrom myos.iso -smp 2 
	)
ADD_CUSTOM_TARGET(
	qd	
	COMMAND qemu-system-i386 -cdrom myos.iso -smp 2 -s -S
	)
ADD_CUSTOM_TARGET(
	cg	
	COMMAND cgdb -command=gdbcommand
	)

add_subdirectory(server)