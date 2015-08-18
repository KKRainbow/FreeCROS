#aux_source_directory(./ SRCS)
set(SRCS Console.cpp TtyIo.cpp Pty.cpp Keyboard.cpp Tty.cpp)

SET(ASM_SRC Keyboard.S)
SET(SRCS ${SRCS} ${ASM_SRC})

SET_SOURCE_FILES_PROPERTIES(${ASM_SRC} PROPERTIES LANGUAGE CXX)
SET(NO_OPTIMIZE Console.cpp)
set_source_files_properties(${NO_OPTIMIZE} PROPERTIES COMPILE_FLAGS "${N_FLAG} -O0")

add_library(tty ${SRCS})
target_link_libraries(tty ${SUPPORT_LIB} arch)
