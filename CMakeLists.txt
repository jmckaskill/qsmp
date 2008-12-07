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
if(UNIX)
  add_definitions(-DUNIX)
endif(UNIX)

set(boost_singleton_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/singleton/)
set(TCL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TCL/)

add_subdirectory(TCL)
add_subdirectory(lua)
#add_subdirectory(luabind)
set(USE_BUILTIN_ID3LIB ${WIN32} CACHE BOOL "Use builtin id3lib - only works on WIN32 atm")
if(USE_BUILTIN_ID3LIB)
	add_subdirectory(id3lib)
endif(USE_BUILTIN_ID3LIB)
if(WIN32)
	add_subdirectory(MMShellHook)
endif(WIN32)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${boost_singleton_INCLUDE_DIR}
                    ${TCL_INCLUDE_DIR})
add_subdirectory(qsmp_lib)
add_subdirectory(qsmp_gui)
add_subdirectory(qsmp_indexer)

