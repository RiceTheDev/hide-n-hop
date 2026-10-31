cmake_minimum_required(VERSION 3.20)

project(hide_and_hop LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.7
    GIT_SHALLOW TRUE
)

FetchContent_GetProperties(imgui)

if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
)

set(ENET_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/enet")

add_library(enet_windows STATIC IMPORTED GLOBAL)

set_target_properties(enet_windows PROPERTIES
    IMPORTED_LOCATION
        "${ENET_ROOT}/build/libenet.a"

    INTERFACE_INCLUDE_DIRECTORIES
        "${ENET_ROOT}/include"
)
if(MINGW)
    add_compile_definitions(
        _GNU_SOURCE
        __USE_MINGW_ANSI_STDIO=1
    )
endif()

set(RAYLIB_ROOT "" CACHE PATH
    "Path to the Windows raylib installation"
)

find_path(RAYLIB_INCLUDE_DIR
    NAMES raylib.h
    PATHS
        "${RAYLIB_ROOT}/include"
        "${RAYLIB_ROOT}/src"
    NO_DEFAULT_PATH
)

find_library(RAYLIB_LIBRARY
    NAMES raylib
    PATHS
        "${RAYLIB_ROOT}/lib"
        "${RAYLIB_ROOT}/lib64"
        "${RAYLIB_ROOT}/src"
    NO_DEFAULT_PATH
)

if(NOT RAYLIB_INCLUDE_DIR OR NOT RAYLIB_LIBRARY)
    message(FATAL_ERROR
        "Windows raylib not found. Set RAYLIB_ROOT to your "
        "Windows raylib installation directory."
    )
endif()

add_library(raylib_windows UNKNOWN IMPORTED GLOBAL)

set_target_properties(raylib_windows PROPERTIES
    IMPORTED_LOCATION "${RAYLIB_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_INCLUDE_DIR}"
)

file(GLOB GAME_C_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c"
)

file(GLOB GAME_CPP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/rlImGui/*.cpp"
)

add_executable(client
    ${GAME_C_SOURCES}
    ${GAME_CPP_SOURCES}
)
if(MINGW)
    target_link_options(client PRIVATE
    	-static
        -static-libgcc
        -static-libstdc++
    )
endif()

set_source_files_properties(
    ${GAME_C_SOURCES}
    PROPERTIES LANGUAGE CXX
)

target_include_directories(client PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/source"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/rlImGui"
)

target_link_libraries(client PRIVATE
    imgui
    raylib_windows
    enet_windows
    m
)

if(WIN32)
    target_link_libraries(client PRIVATE
        ws2_32
        winmm
        gdi32
        opengl32
    )
endif()
