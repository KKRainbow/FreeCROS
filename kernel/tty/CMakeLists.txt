#aux_source_directory(./ SRCS)
set(SRCS Console.cpp TtyIo.cpp Pty.cpp Keyboard.cpp Tty.cpp)

SET(ASM_SRC Keyboard.S)
SET(SRCS ${SRCS} ${ASM_SRC})

SET_SOURCE_FILES_PROPERTIES(${ASM_SRC} PROPERTIES LANGUAGE C)

add_library(tty ${SRCS})
target_link_libraries(tty ${SUPPORT_LIB} arch)
