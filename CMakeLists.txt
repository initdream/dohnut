cmake_minimum_required(VERSION 3.6...4.0)

project(SRR2)

option(BUILD_STATIC "Build completely static executable" OFF)
if(BUILD_STATIC)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    message(STATUS "STATIC BUILD ENABLED")
endif()

set(SRR2_TITLE "Simpsons Hit & Run")
set(SRR2_VERSION "00.61")
set(SRR2_FILENAME "Simpsons")
set(SRR2_REGION "NTSC" CACHE STRING "Select PAL for the EFIGS multi-language version")
set_property(CACHE SRR2_REGION PROPERTY STRINGS "NTSC" "PAL")

option(SRR2_USE_DLMALLOC "Use Doug Lea's memory allocator for fixed-size heaps" OFF)
option(SRR2_REROUTE_ALL_HEAP_ALLOCATIONS "Reroute all memory allocations by new operator to internal heaps" OFF)
option(SRR2_USE_BINK "Use RAD Game Tools Bink SDK instead of FFmpeg" OFF)
option(SRR2_BUILD_TESTS "Build sample programs for internal libraries" ON)
option(SRR2_MICROPROFILE "Use microprofile instead of the internal profiler" OFF)
option(SRR2_P3D_CG "Use CG shaders rather than GLSL for OpenGL ES" OFF)
set(SRR2_P3D_PDDI "OpenGL" CACHE STRING "Pure3D PDDI implementation")

if(VITA)
    option(SRR2_USE_VITAGL "Use VitaGL as an OpenGL (ES) implementation" ON)
    set_property(CACHE SRR2_P3D_PDDI PROPERTY STRINGS "OpenGL" "GLES1" "GLES2" "GXM")
    include("${VITASDK}/share/vita.cmake" REQUIRED)
    add_definitions(-DRAD_VITA)
    
    set(VITA_APP_NAME ${SRR2_TITLE})
    set(VITA_TITLEID  "ROADRAGE2")
    set(VITA_VERSION ${SRR2_VERSION})
else()
    set_property(CACHE SRR2_P3D_PDDI PROPERTY STRINGS "OpenGL" "GLES1" "GLES2")
endif()

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

find_package(SDL2 QUIET)

if(NOT SDL2_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
endif()


if(NOT TARGET SDL2::SDL2)
    add_library(SDL2::SDL2 INTERFACE IMPORTED)
    set_target_properties(SDL2::SDL2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SDL2_LIBRARIES}"
        INTERFACE_COMPILE_OPTIONS "${SDL2_CFLAGS_OTHER}"
    )
endif()

if(NOT TARGET SDL2::SDL2main)
    add_library(SDL2::SDL2main INTERFACE IMPORTED)
    set_target_properties(SDL2::SDL2main PROPERTIES INTERFACE_LINK_LIBRARIES SDL2::SDL2)
endif()

set(SDL_LIBRARY SDL2::SDL2 SDL2::SDL2main)

find_package(PNG REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenAL REQUIRED)

if(SRR2_USE_BINK)
    add_definitions(-DRAD_MOVIEPLAYER_USE_BINK)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFmpeg REQUIRED IMPORTED_TARGET
            libavformat libavcodec libavutil libswresample libswscale)
    if(FFmpeg_libavformat_VERSION VERSION_GREATER_EQUAL "59")
        message(STATUS "Modern FFmpeg detected (libavformat ${FFmpeg_libavformat_VERSION}). Using new API.")
        add_compile_definitions(RAD_FFMPEG_NEW_API)
    else()
        message(STATUS "Legacy FFmpeg (< 5.0) detected. Using old API.")
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fvisibility=hidden)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-ggdb)
    endif()
    if(VITA)
        add_compile_options(-mtune=cortex-a9 -mfpu=neon)
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DRAD_DEBUG)
else()
    add_definitions(-DRAD_RELEASE)
endif()

add_definitions(-DRAD_CONSOLE -DRAD_WIN32 -DRAD_SDL)

if(SRR2_REGION STREQUAL "PAL")
    add_definitions(-DPAL)
endif()
if(SRR2_USE_DLMALLOC)
    add_definitions(-DUSE_DOUG_LEA_HEAP)
endif()
if(SRR2_REROUTE_ALL_HEAP_ALLOCATIONS)
    add_definitions(-DREROUTE_ALL_HEAP_ALLOCATIONS)
endif()
if(SRR2_MICROPROFILE)
    add_definitions(-DMICROPROFILE_ENABLED)
endif()
if(SRR2_P3D_CG)
    add_definitions(-DRAD_CG)
endif()
if(SRR2_USE_VITAGL)
    add_definitions(-DRAD_VITAGL)
endif()
if(SRR2_P3D_PDDI STREQUAL "GXM")
    add_definitions(-DRAD_GXM)
endif()
if(SRR2_P3D_PDDI STREQUAL "GLES1")
    add_definitions(-DRAD_GLES -DRAD_GLES_VERSION=1)
endif()
if(SRR2_P3D_PDDI STREQUAL "GLES2")
    add_definitions(-DRAD_GLES -DRAD_GLES_VERSION=2)
endif()
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(libs)
add_subdirectory(code)

if(VITA)
    if(SRR2_P3D_PDDI STREQUAL "GLES2" AND NOT SRR2_USE_VITAGL)
        vita_create_self(${PROJECT_NAME}.self code/${PROJECT_NAME} UNSAFE)
        vita_create_vpk(${SRR2_FILENAME}${SRR2_REGION}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
            VERSION ${VITA_VERSION} NAME ${VITA_APP_NAME}
            FILE sce_sys/icon0.png sce_sys/icon0.png
            sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
            sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
            sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
            art art module/libGLESv2.suprx module/libGLESv2.suprx
            module/libgpu_es4_ext.suprx module/libgpu_es4_ext.suprx
            module/libIMGEGL.suprx module/libIMGEGL.suprx
            module/libpvrPSP2_WSEGL.suprx module/libpvrPSP2_WSEGL.suprx
        )
    else()
        vita_create_self(${PROJECT_NAME}.self code/${PROJECT_NAME})
        vita_create_vpk(${SRR2_FILENAME}${SRR2_REGION}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
            VERSION ${VITA_VERSION} NAME ${VITA_APP_NAME}
            FILE sce_sys/icon0.png sce_sys/icon0.png
            sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
            sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
            sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
            art art libs/pure3d/pddi/gxm/shaders/vertex_cg.gxp shaders/vertex_cg.gxp
            libs/pure3d/pddi/gxm/shaders/color_cg.gxp shaders/color_cg.gxp
            libs/pure3d/pddi/gxm/shaders/texture_cg.gxp shaders/texture_cg.gxp
            libs/pure3d/pddi/gxm/shaders/alpha_cg.gxp shaders/alpha_cg.gxp
        )
    endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
    nx_generate_nacp(OUTPUT ${PROJECT_NAME}.nacp NAME ${SRR2_TITLE}
        AUTHOR "Radical Entertainment" VERSION ${SRR2_VERSION})
    nx_create_nro(code/${PROJECT_NAME} OUTPUT ${SRR2_FILENAME}${SRR2_REGION}.nro
        ICON ${PROJECT_NAME}.jpg NACP ${PROJECT_NAME}.nacp ROMFS romfs)
endif()