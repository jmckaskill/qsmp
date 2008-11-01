project(qsmp)

#TODO: Boost and TBB have static paths for now ... will change to
#paths set from ccmake when I get around to it
set(BOOST_ROOT E:/SCM/boost_1_36_0)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS filesystem system)

set(TBB_ROOT E:/SCM/tbb-win32)
set(TBB_INCLUDE_DIR ${TBB_ROOT}/include)
set(TBB_LIBRARY_DIR ${TBB_ROOT}/ia32/vc8/lib)
set(TBB_LIBRARIES debug ${TBB_LIBRARY_DIR}/tbb_debug.lib
                  debug ${TBB_LIBRARY_DIR}/tbbmalloc_debug.lib
                  optimized ${TBB_LIBRARY_DIR}/tbb.lib
                  optimized ${TBB_LIBRARY_DIR}/tbbmalloc.lib)


include_directories(${Boost_INCLUDE_DIR} 
                    ${MMShellHook_SOURCE_DIR}
		    ${id3lib_INCLUDE_DIR}
		    ${lua_INCLUDE_DIR}
		    ${TBB_INCLUDE_DIR}
		    ${boost_singleton_INCLUDE_DIR})

add_definitions(-DID3LIB_LINKOPTION=1
                -DQT_NO_KEYWORDS)

set(QT_USE_PHONON 1)
set(QT_USE_QTNETWORK 1)
find_package(Qt4)
include(${QT_USE_FILE})

set(moc_headers src/LuaTcpConsole.h
                src/PlaylistModel.h
                src/Player.h
                src/PlaylistView.h)
set(ui ui/untitled.ui)
set(headers src/common.h
            src/utilities.h
            src/Playlist.h
	    src/PlaylistManager.h
	    src/LuaRegister.h)
set(sources src/main.cpp
            src/LuaTcpConsole.cpp
            src/PlaylistModel.inl
            src/LuaRegister.cpp
            src/Player.cpp
            src/PlaylistView.cpp)

qt4_wrap_cpp(moc_sources ${moc_headers})
qt4_wrap_ui(moc_sources ${ui})
add_executable(qsmp ${sources} ${moc_headers} ${headers} ${ui} ${moc_sources})
message(${Boost_LIBRARIES})
target_link_libraries(qsmp 
                      ${QT_LIBRARIES}
		      ${Boost_LIBRARIES}
		      ${TBB_LIBRARIES}
		      MMShellHook
		      id3lib
		      zlib
		      lua
		      luabind)

