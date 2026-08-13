cmake_minimum_required(VERSION 3.15)
project(FolderSizeExt C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include MinHook
include_directories(minhook/include)

set(MINHOOK_SOURCES
    minhook/src/buffer.c
    minhook/src/hook.c
    minhook/src/trampoline.c
    minhook/src/hde/hde32.c
    minhook/src/hde/hde64.c
)

add_library(FolderSizeExt SHARED
    FolderSizeExt.cpp
    EverythingClient.cpp
    ${MINHOOK_SOURCES}
)

target_link_libraries(FolderSizeExt PRIVATE
    dbghelp
    propsys
    shlwapi
    ole32
)

set_target_properties(FolderSizeExt PROPERTIES
    PREFIX ""
    OUTPUT_NAME "FolderSizeExt"
)
