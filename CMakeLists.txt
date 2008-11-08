cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

project(SMP)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/id3lib/libprj 
                 ${CMAKE_CURRENT_SOURCE_DIR}/id3lib/zlib/prj)
add_definitions(-D_WIN32_WINNT=0x500)

set(boost_singleton_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/singleton/)
set(TCL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TCL/)

add_subdirectory(TCL)
add_subdirectory(lua)
add_subdirectory(luabind)
add_subdirectory(id3lib)
add_subdirectory(MediaPlayer)
add_subdirectory(MMShellHook)

