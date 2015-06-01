cmake_minimum_required(VERSION 2.6)
project(freecros)

SET(ARCHITECT "x86")

SET(CMAKE_CXX_FLAGS "-fomit-frame-pointer -g -O0 -ffreestanding -fno-builtin -fno-rtti -std=c++11 -m32 -fno-exceptions -nostdlib -nodefaultlibs -nostartfiles -nostdinc -nostdinc++")
SET(CMAKE_C_FLAGS "-fomit-frame-pointer -g -O0 -m32 -ffreestanding -fno-builtin -nostdlib -nodefaultlibs -nostartfiles -nostdinc ")

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



add_subdirectory(kernel)
add_subdirectory(config)
add_subdirectory(lib)
add_subdirectory(test)