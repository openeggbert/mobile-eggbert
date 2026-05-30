cmake_minimum_required(VERSION 3.21)
project(WindowsPhoneSpeedyBlupi LANGUAGES C CXX)

message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

if (NOT WIN32 AND CMAKE_BINARY_DIR MATCHES "build-windows")
    message(WARNING "Building in 'build-windows' directory, but WIN32 is not set. "
                    "Make sure you used -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64.cmake "
                    "on a CLEAN build directory.")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable sprite-batch grouping in Pixmap: all Draw calls between BeginBatch/
# EndBatch in Game1::Draw share a single SpriteBatch Begin/End pair instead of
# one per sprite.  Reduces GL draw calls by ~50-100x.  OFF by default so that
# the original Daniel Roux per-sprite behaviour is preserved until the feature
# has been thoroughly tested.
option(CNA_SPRITE_BATCHING_ENABLED "Group all Pixmap draws into one SpriteBatch Begin/End per frame" OFF)

# On Emscripten and Android SDL_RENDERER is the only supported backend
# (no OpenGL extensions / desktop GL). For native Linux/Windows either
# EASY_GL or SDL_RENDERER can be used; default to SDL_RENDERER so the
# same backend works on all targets.
if(EMSCRIPTEN OR ANDROID)
    set(CNA_BACKEND_SDL_RENDERER OFF  CACHE BOOL "Enable SDL_Renderer backend in CNA" FORCE)
    set(CNA_BACKEND_EASY_GL      ON CACHE BOOL "Enable easy-gl backend in CNA"      FORCE)
else()
    set(CNA_BACKEND_SDL_RENDERER OFF  CACHE BOOL "Enable SDL_Renderer backend in CNA")
    set(CNA_BACKEND_EASY_GL      ON CACHE BOOL "Enable easy-gl backend in CNA")
endif()
set(CNA_BACKEND_BGFX   OFF CACHE BOOL "Enable bgfx backend in CNA")
set(CNA_BACKEND_VULKAN OFF CACHE BOOL "Enable Vulkan backend in CNA")
set(CNA_GRAPHICS_BACKEND "EASYGL" CACHE STRING "Selected CNA backend (SDL_RENDERER, EASYGL, BGFX, VULKAN)")

add_subdirectory(../cna CNA)
include_directories(cna/include CNA)

file(GLOB_RECURSE SOURCES "src/*.cpp")

# On Android the "executable" is a shared library loaded by SDLActivity.
# SDL's Java glue expects the native library to be named "libmain.so".
# We use a CMake variable so all subsequent target_* calls use the right name.
if(ANDROID)
    set(_game_target main)
    add_library(${_game_target} SHARED ${SOURCES}
            include/WindowsPhoneSpeedyBlupi/def/Direction.hpp
            ../cna/include/CNA/Misc.hpp)
else()
    set(_game_target WindowsPhoneSpeedyBlupi)
    add_executable(${_game_target} ${SOURCES})
endif()
# On Emscripten the output is HTML+JS+WASM; set the suffix accordingly.
if(EMSCRIPTEN)
    set_target_properties(${_game_target} PROPERTIES SUFFIX ".html")
endif()

target_include_directories(${_game_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(CNA_SPRITE_BATCHING_ENABLED)
    target_compile_definitions(${_game_target} PRIVATE CNA_SPRITE_BATCHING_ENABLED)
endif()

# SDL3 is provided by CNA via vendored submodules (or system SDL if CNA_USE_SYSTEM_SDL=ON).
# No separate find_package(SDL3) call is needed here.

# CNA and the selected graphics backend static library have circular
# symbol references on Linux (backend uses Color/Rectangle/Matrix from CNA,
# CNA uses backend interfaces). Wrap them in a linker group so GNU ld
# resolves both without depending on link order.
# Note: --start-group/--end-group are not available on Emscripten's linker
# or on Android (lld does not need it and warns).
if(EMSCRIPTEN)
    target_link_libraries(${_game_target} PRIVATE CNA SDL3::SDL3-static)
elseif(ANDROID)
    # Android uses lld which resolves circular static-lib references without
    # --start-group. On Android the executable is actually a shared library
    # (libmain.so) loaded by SDLActivity; SDL3 is linked statically.
    target_link_libraries(${_game_target} PRIVATE CNA SDL3::SDL3 android log)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT WIN32)
    set(_cna_backend_target "")
    if(TARGET cna_backend_graphics_easygl)
        set(_cna_backend_target cna_backend_graphics_easygl)
    elseif(TARGET cna_backend_graphics_sdl_renderer)
        set(_cna_backend_target cna_backend_graphics_sdl_renderer)
    elseif(TARGET cna_backend_graphics_bgfx)
        set(_cna_backend_target cna_backend_graphics_bgfx)
    elseif(TARGET cna_backend_graphics_vulkan)
        set(_cna_backend_target cna_backend_graphics_vulkan)
    endif()
    if(_cna_backend_target)
        target_link_libraries(${_game_target} PRIVATE
            -Wl,--start-group CNA ${_cna_backend_target} -Wl,--end-group
            SDL3::SDL3)
    else()
        target_link_libraries(${_game_target} PRIVATE CNA SDL3::SDL3)
    endif()
else()
    target_link_libraries(${_game_target} PRIVATE CNA SDL3::SDL3)
endif()

if(TARGET SDL3::SDL3main)
    target_link_libraries(${_game_target} PRIVATE SDL3::SDL3main)
endif()

if(ANDROID)
    # ---------------------------------------------------------------------------
    # Android: no content copy needed (assets are in the APK).
    # ---------------------------------------------------------------------------
elseif(EMSCRIPTEN)
    # ---------------------------------------------------------------------------
    # Emscripten link options
    # ---------------------------------------------------------------------------
    # Asset virtual-filesystem layout (mounted paths match what the game expects
    # at runtime relative to the working directory):
    #
    #   ${CMAKE_SOURCE_DIR}/Content/backgrounds  ->  /Content/backgrounds
    #   ${CMAKE_SOURCE_DIR}/Content/icons        ->  /Content/icons
    #   ${CMAKE_SOURCE_DIR}/Content/sounds       ->  /Content/sounds
    #   ${CMAKE_SOURCE_DIR}/worlds               ->  /worlds
    #
    # Save data is persisted via IDBFS mounted at /save (see StoragePaths.cpp).
    # The JS shell code mounts IDBFS after Module init and before the game loop.
    target_link_options(${_game_target} PRIVATE
        -sALLOW_MEMORY_GROWTH=1
        -sSTACK_SIZE=1048576
        -sINITIAL_MEMORY=134217728
        -sFORCE_FILESYSTEM=1
        "-sEXPORTED_RUNTIME_METHODS=['FS','ccall','cwrap']"
        -lidbfs.js
        "SHELL:--pre-js ${CMAKE_SOURCE_DIR}/cmake/web/pre.js"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/Content/backgrounds@/Content/backgrounds"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/Content/icons@/Content/icons"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/Content/sounds@/Content/sounds"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/worlds@/worlds"
        "-sMIN_WEBGL_VERSION=2"
        "-sMAX_WEBGL_VERSION=2"
    )
else()
    # Native desktop: copy Content directory next to the executable.
    set(MOBILE_EGGBERT_CONTENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Content")
    set(MOBILE_EGGBERT_WORLDS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/worlds")
    if(EXISTS "${MOBILE_EGGBERT_CONTENT_DIR}")
        add_custom_command(TARGET ${_game_target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${MOBILE_EGGBERT_CONTENT_DIR}"
                "$<TARGET_FILE_DIR:${_game_target}>/Content"
                COMMENT "Copying game content next to executable")
    endif()
#    if(EXISTS "${MOBILE_EGGBERT_WORLDS_DIR}")
#        add_custom_command(TARGET ${_game_target} POST_BUILD
#                COMMAND ${CMAKE_COMMAND} -E copy_directory
#                "${CMAKE_CURRENT_WORLDS_DIR}"
#                "$<TARGET_FILE_DIR:${_game_target}>/worlds"
#                COMMENT "Copying game worlds next to executable")
#    endif()
endif() # ANDROID / EMSCRIPTEN / desktop

if(WIN32)
    cna_copy_sdl_runtime(${_game_target})
endif()

if(MINGW)
    # Statically link the MinGW GCC and C++ runtime so the exe runs outside
    # CLion/MSYS2 without needing libgcc_s_seh-1.dll / libstdc++-6.dll.
    target_link_options(${_game_target} PRIVATE -static-libgcc -static-libstdc++)
    # libwinpthread-1.dll cannot be statically linked this way; copy it instead.
    cna_copy_mingw_runtime(${_game_target})
endif()

find_package(Doxygen)
if (DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
endif ()
