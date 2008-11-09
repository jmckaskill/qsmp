project(qsmp)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS filesystem system thread date_time)

set(TBB_ROOT E:/SCM/tbb-win32 CACHE FILEPATH "tbb root")
set(TBB_INCLUDE_DIR ${TBB_ROOT}/include)
set(TBB_LIBRARY_DIR ${TBB_ROOT}/ia32/vc8/lib CACHE FILEPATH "tbb lib dir")
set(TBB_LIBRARIES debug ${TBB_LIBRARY_DIR}/tbb_debug.lib
                  debug ${TBB_LIBRARY_DIR}/tbbmalloc_debug.lib
                  optimized ${TBB_LIBRARY_DIR}/tbb.lib
                  optimized ${TBB_LIBRARY_DIR}/tbbmalloc.lib)


include_directories(${Boost_INCLUDE_DIR} 
                    ${MMShellHook_SOURCE_DIR}
                    ${id3lib_INCLUDE_DIR}
                    ${lua_INCLUDE_DIR}
                    ${TBB_INCLUDE_DIR}
                    ${TCL_INCLUDE_DIR}
                    ${boost_singleton_INCLUDE_DIR}
                    include
                    ${CMAKE_CURRENT_BINARY_DIR}/moc)

add_definitions(-DID3LIB_LINKOPTION=1
                -DQT_NO_KEYWORDS)

set(QT_USE_PHONON 1)
set(QT_USE_QTNETWORK 1)
find_package(Qt4)
include(${QT_USE_FILE})

set(ui ui/untitled.ui)
set(headers include/qsmp/common.h
            include/qsmp/utilities.h
            include/qsmp/Playlist.h
            include/qsmp/PlaylistManager.h
            include/qsmp/PlaylistModel.inl
            include/qsmp/PlaylistModel.h
            include/qsmp/LuaRegister.h
            include/qsmp/Log.h
            include/qsmp/LuaTcpConsole.h
            include/qsmp/Player.h
            include/qsmp/ViewSelector.h
            include/qsmp/PlaylistView.h
            include/qsmp/HotkeyWindow.h
            src/stdafx.h)
set(sources src/main.cpp
            src/LuaTcpConsole.cpp
            src/LuaRegister.cpp
            src/Player.cpp
            src/Log.cpp
            src/PlaylistView.cpp
            src/HotkeyWindow.cpp
            src/PlaylistModel.cpp
            src/stdafx.cpp)

qsmp_qt4_automoc(${sources})
qt4_wrap_ui(moc_sources ${ui})
add_executable(qsmp ${sources} ${headers} ${ui})

set_precompiled_header(qsmp stdafx.h src/stdafx.cpp)

target_link_libraries(qsmp 
                      ${QT_LIBRARIES}
                      ${Boost_LIBRARIES}
                      ${TBB_LIBRARIES}
                      MMShellHook
                      id3lib
                      zlib
                      lua
                      luabind)
