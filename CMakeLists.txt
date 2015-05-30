cmake_minimum_required(VERSION 2.6)
project(freecros)

add_executable(freecros main.cpp)

install(TARGETS freecros RUNTIME DESTINATION bin)
