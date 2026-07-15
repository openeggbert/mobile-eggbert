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

# -----------------------------------------------------------------------------
# Graphics backend selection.
# -----------------------------------------------------------------------------
# CNA_GRAPHICS_BACKEND is the only thing that needs setting -- it's a single
# string, and cna's own CMakeLists.txt keys every downstream decision
# (which backend sources to compile, which target to create, which sibling
# deps to pull in) off it directly.
#
# cna also exposes a parallel set of CNA_BACKEND_* boolean options,
# but those are an *optional, alternative* input path: they all default to
# OFF, and cna only consults them if one is explicitly switched ON,
# in which case it derives CNA_GRAPHICS_BACKEND from the boolean instead. A
# consumer that just sets the string never needs to touch them.
#
# This file used to force all of CNA_BACKEND_*'s 5-and-growing flags itself
# (~75 lines of if/elseif) -- pure redundancy with the string above, and it
# hardcoded a fixed backend list that silently fell out of sync every time
# cna added one (HEADLESS had already been bolted on as a partial
# exception; SOFTWARE and D3D11, which cna also defines, were never
# added at all). Replaced with: set the string, validate it against a live
# list, done.
set(MOBILE_EGGBERT_BACKENDS SDL_RENDERER EASYGL BGFX VULKAN WEBGPU HEADLESS SOFTWARE D3D11 D3D12 CANVAS ASCII)

if(ANDROID)
    # Only SDL_RENDERER is supported (no desktop GL context the same way;
    # SDL_RENDERER runs on GLES under the hood).
    set(CNA_GRAPHICS_BACKEND "SDL_RENDERER" CACHE STRING "Selected CNA backend" FORCE)
elseif(EMSCRIPTEN)
    # SDL_RENDERER (runs on WebGL under the hood) is the default and most
    # tested; CANVAS (browser HTML5 Canvas 2D, GPU-free) can be selected
    # explicitly with -DCNA_GRAPHICS_BACKEND=CANVAS -- see cna/plan_canvas.md.
    set(CNA_GRAPHICS_BACKEND "SDL_RENDERER" CACHE STRING "Selected CNA backend")
else()
    # Desktop: choose one backend, default SDL_RENDERER. Override with
    # -DCNA_GRAPHICS_BACKEND=<backend>, e.g. -DCNA_GRAPHICS_BACKEND=VULKAN.
    # WEBGPU is experimental (see cna/plan_webgpu.md). HEADLESS
    # touches no GPU/window at all (see cna/plan_headless.md) -- for
    # driving this game's own logic under CI without a display server, not
    # for visible rendering. ASCII is an SDL-windowed glyph-grid decorator,
    # not a real terminal/TTY backend (see cna/plan_ascii.md).
    set(CNA_GRAPHICS_BACKEND "ASCII" CACHE STRING "Selected CNA backend")
endif()
set_property(CACHE CNA_GRAPHICS_BACKEND PROPERTY STRINGS ${MOBILE_EGGBERT_BACKENDS})

if(NOT CNA_GRAPHICS_BACKEND IN_LIST MOBILE_EGGBERT_BACKENDS)
    message(FATAL_ERROR
        "mobile-eggbert: unknown CNA_GRAPHICS_BACKEND='${CNA_GRAPHICS_BACKEND}'. "
        "Choose one of: ${MOBILE_EGGBERT_BACKENDS}.")
endif()

set(CNA_GRAPHICS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cna")
add_subdirectory("${CNA_GRAPHICS_SOURCE_DIR}" CNA)
include_directories("${CNA_GRAPHICS_SOURCE_DIR}/include" CNA)

file(GLOB_RECURSE SOURCES "src/*.cpp")

# On Android the "executable" is a shared library loaded by SDLActivity.
# SDL's Java glue expects the native library to be named "libmain.so".
# We use a CMake variable so all subsequent target_* calls use the right name.
if(ANDROID)
    set(_game_target main)
    add_library(${_game_target} SHARED ${SOURCES}
            include/WindowsPhoneSpeedyBlupi/def/Direction.hpp
            "${CNA_GRAPHICS_SOURCE_DIR}/include/CNA/Misc.hpp")
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

# CNA builds SDL3 once into its own prebuilt root and sets SDL3_DIR / SDL3_image_DIR /
# SDL3_mixer_DIR in the CMake cache.  SDL IMPORTED targets are directory-scoped, so we
# must call find_package() here to make SDL3::SDL3 visible in this project's scope.
find_package(SDL3       REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)
find_package(SDL3_mixer REQUIRED CONFIG)

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
    # cna names every backend target
    # cna_backend_graphics_<backend, lowercased> -- this used to be a
    # hardcoded if/elseif chain of 5 backend names (in existence-check order,
    # not selection order) that had silently fallen behind cna's own
    # backend list: HEADLESS was missing from it entirely, so selecting
    # HEADLESS here fell through to the plain (non-grouped) link below,
    # skipping the --start-group/--end-group circular-symbol workaround this
    # branch exists for. Deriving the name directly from CNA_GRAPHICS_BACKEND
    # fixes that and covers every future backend automatically.
    string(TOLOWER "${CNA_GRAPHICS_BACKEND}" _cna_backend_lower)
    set(_cna_backend_target "cna_backend_graphics_${_cna_backend_lower}")
    if(TARGET ${_cna_backend_target})
        target_link_libraries(${_game_target} PRIVATE
            -Wl,--start-group CNA CNA_GamerServices ${_cna_backend_target} -Wl,--end-group
            SDL3::SDL3)
    else()
        target_link_libraries(${_game_target} PRIVATE CNA CNA_GamerServices SDL3::SDL3)
    endif()
else()
    target_link_libraries(${_game_target} PRIVATE CNA CNA_GamerServices SDL3::SDL3)
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
    if(CNA_GRAPHICS_BACKEND STREQUAL "WEBGPU" AND CNA_WEBGPU_RUNTIME_LIBRARY)
        add_custom_command(TARGET ${_game_target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CNA_WEBGPU_RUNTIME_LIBRARY}" "$<TARGET_FILE_DIR:${_game_target}>"
                COMMENT "Copying wgpu-native runtime next to game executable")
        if(UNIX AND NOT APPLE)
            set_property(TARGET ${_game_target} APPEND PROPERTY BUILD_RPATH "$ORIGIN")
        endif()
    endif()
    if(EXISTS "${MOBILE_EGGBERT_WORLDS_DIR}")
        add_custom_command(TARGET ${_game_target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${MOBILE_EGGBERT_WORLDS_DIR}"
                "$<TARGET_FILE_DIR:${_game_target}>/worlds"
                COMMENT "Copying game worlds next to executable")
    endif()
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
