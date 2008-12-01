cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
include(cmake/Qt4Hacks.cmake)
include(cmake/PrecompiledHeaders.cmake)

project(SMP)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/id3lib/libprj 
                 ${CMAKE_CURRENT_SOURCE_DIR}/id3lib/zlib/prj)

if(WIN32)
	add_definitions(-D_WIN32_WINNT=0x500
	                -D_CRT_SECURE_NO_WARNINGS
	                -D_SCL_SECURE_NO_WARNINGS)
endif(WIN32)

set(boost_singleton_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/singleton/)
set(TCL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TCL/)

add_subdirectory(TCL)
add_subdirectory(lua)
add_subdirectory(luabind)
add_subdirectory(id3lib)
add_subdirectory(MMShellHook)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${boost_singleton_INCLUDE_DIR}
                    ${TCL_INCLUDE_DIR})
add_subdirectory(qsmp_lib)
add_subdirectory(qsmp_gui)
add_subdirectory(qsmp_indexer)

