#PROJECT(LalrLib)
#CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

FILE(GLOB_RECURSE BasicSrcs ./Lalrlib/library/basic/*.cpp)
FILE(GLOB_RECURSE SstlSrcs ./Lalrlib/library/stl/*.cpp)

ADD_DEFINITIONS("-D__CROS")

#SET(CMAKE_CXX_FLAGS "-std=c++11 " ${CMAKE_CXX_FLAGS})

ADD_LIBRARY(lalr ${BasicSrcs} ${SstlSrcs})
