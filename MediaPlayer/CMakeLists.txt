project(MediaPlayer)

include_directories($MMShellHook_SOURCE_DIR)
add_definitions(-DID3LIB_LINKOPTION=1)

find_package(Qt4)
set(QT_USE_PHONON 1)
include(${QT_USE_FILE})

set(headers src/common.h
            src/PlaybackControls.h
            src/utilities.h
            src/MainWin.h)
set(ui ui/untitled.ui)

set(sources src/main.cpp
            src/PlaybackControls_Phonon.cpp
            src/MainWin.cpp)

add_executable(mediaplayer ${sources} ${headers} ${ui})
qt4_wrap_cpp(mediaplayer ${headers})
qt4_wrap_ui(mediaplayer ${ui})
target_link_libraries(mediaplayer ${QT_LIBRARIES} MMShellHook)

