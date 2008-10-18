project(MMShellHook)

add_definitions(-DMMSHELLHOOK_EXPORTS -D_USRDLL -D_WIN32_WINNT=0x500)
add_library(MMShellHook SHARED MMShellHook.cpp MMShellHook.h MMShellHook.def MMShellHook.rc)
