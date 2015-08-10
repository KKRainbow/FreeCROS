add_subdirectory(keyboard)
# add_subdirectory(tty)

FILE(GLOB SRCS ./*.cpp)
ADD_EXECUTABLE(device ${SRCS})
TARGET_LINK_LIBRARIES(device c c++ lkernel keyboard server)
