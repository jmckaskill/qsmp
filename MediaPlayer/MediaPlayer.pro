
TEMPLATE = app
TARGET = 
DEPENDPATH += . src ui
INCLUDEPATH += . D:\Data\Projects\id3lib-3.8.3\include \
                 D:\Data\Projects\id3lib-3.8.3\include\id3 \
                 D:\Data\Projects\mediakeys\MMShellHook
DEFINES += ID3LIB_LINKOPTION=1

QT += phonon

# Input
HEADERS += src/common.h src/PlaybackControls.h src/utilities.h src/MainWin.h
FORMS += ui/untitled.ui
SOURCES += src/main.cpp \
           src/PlaybackControls_Phonon.cpp \
           src/MainWin.cpp
           
PRECOMPILED_HEADER = common.h
UI_HEADERS_DIR = src
CONFIG += embed_manifest_exe

CONFIG(ReleaseBuild) {
LIBS += D:\Data\Projects\mediakeys\_Release\MMShellHook.lib D:\Data\Projects\id3lib-3.8.3\libprj\id3lib.lib
}

CONFIG(DebugBuild) {
LIBS += D:\Data\Projects\mediakeys\_Debug\MMShellHook.lib D:\Data\Projects\id3lib-3.8.3\libprj\id3libD.lib
}