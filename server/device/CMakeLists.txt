# add_subdirectory(keyboard)
add_subdirectory(tty)

FILE(GLOB_RECURSE SRCS ./*.cpp)
ADD_EXECUTABLE(device ${SRCS})
TARGET_LINK_LIBRARIES(device c c++ lkernel keyboard)
