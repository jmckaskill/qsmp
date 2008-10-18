project(qsmp)

set(BOOST_ROOT E:/SCM/boost_1_36_0)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost)

include_directories(${Boost_INCLUDE_DIR} 
                    ${MMShellHook_SOURCE_DIR}
		    ${id3lib_INCLUDE_DIR}
		    ${lua_INCLUDE_DIR}
		    ${luabind_INCLUDE_DIR})

add_definitions(-DID3LIB_LINKOPTION=1)

set(QT_USE_PHONON 1)
set(QT_USE_QTNETWORK 1)
find_package(Qt4)
include(${QT_USE_FILE})

set(moc_headers src/PlaybackControls.h
                src/MainWin.h
                src/LuaTcpConsole.h)
set(ui ui/untitled.ui)
set(headers src/common.h
            src/utilities.h
            src/Playlist.h)
set(sources src/main.cpp
            src/PlaybackControls_Phonon.cpp
            src/MainWin.cpp
            src/LuaTcpConsole.cpp)

qt4_wrap_cpp(moc_sources ${moc_headers})
qt4_wrap_ui(moc_sources ${ui})
add_executable(qsmp ${sources} ${moc_headers} ${headers} ${ui} ${moc_sources})
target_link_libraries(qsmp 
                      ${QT_LIBRARIES}
		      ${Boost_LIBRARIES}
		      MMShellHook
		      id3lib
		      zlib
		      lua
		      luabind)

