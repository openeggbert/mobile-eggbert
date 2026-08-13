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

# CNA owns the compile-time renderer registry, platform validation and defaults. Consumers select
# one of its 46 renderer identities with -DCNA_GRAPHICS_RENDERER=<name>. Keeping the registry in
# one place means a new CNA renderer is immediately available to Mobile Eggbert without another
# hard-coded list here falling out of sync.

# CNA's own unit tests and demo/example applications are not part of this game and pull in
# hundreds of extra translation units. A plain (non-cache) set() turns them off because CMP0077
# is NEW at cmake_minimum_required(3.21), so CNA's option() calls honour it.
option(MOBILE_EGGBERT_BUILD_CNA_TESTS "Also build cna's own unit tests and example apps" OFF)
if(NOT MOBILE_EGGBERT_BUILD_CNA_TESTS)
    set(CNA_BUILD_TESTS OFF)
    set(CNA_BUILD_EXAMPLES OFF)
endif()

# CNA currently composes GamerServices and Net behind one switch. The game needs Guide from
# GamerServices, so composition must stay enabled; the Net target is not linked into the game.
set(CNA_ENABLE_NET ON)

# CNA's selected renderer currently declares this framework-wide sharp-runtime component set.
# Mobile Eggbert additionally uses isolated storage directly for save games. Selecting this
# closure prevents sharp-runtime's add_subdirectory fallback from enabling all optional modules.
set(SHARP_RUNTIME_COMPONENTS
    "Core.Base;IO;Collections.Core;Collections.ObjectModel;Runtime;Threading;Text;Globalization;Storage;IO.IsolatedStorage"
    CACHE STRING "Sharp Runtime components required by Mobile Eggbert and CNA")

set(MOBILE_EGGBERT_CNA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../cna" CACHE PATH
    "Path to the CNA checkout consumed by Mobile Eggbert")
if(NOT EXISTS "${MOBILE_EGGBERT_CNA_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR
        "mobile-eggbert: CNA checkout not found at '${MOBILE_EGGBERT_CNA_ROOT}'. "
        "Clone CNA next to this repository or set -DMOBILE_EGGBERT_CNA_ROOT=<path>.")
endif()
add_subdirectory("${MOBILE_EGGBERT_CNA_ROOT}" CNA)
# No include_directories() for CNA here: each modules/<name>/ owns its include root, and the
# module targets below propagate the required include paths through their dependency closure.

file(GLOB_RECURSE SOURCES "src/*.cpp")

# On Android the "executable" is a shared library loaded by SDLActivity.
# SDL's Java glue expects the native library to be named "libmain.so".
# We use a CMake variable so all subsequent target_* calls use the right name.
if(ANDROID)
    set(_game_target main)
    add_library(${_game_target} SHARED ${SOURCES}
            include/WindowsPhoneSpeedyBlupi/def/Direction.hpp
            # CNA/Misc.hpp moved to modules/runtime/ with cna's physical module layout; the old
            # include/CNA/Misc.hpp path no longer exists and made this branch fail with
            # "Cannot find source file" on every Android configure.
            "${MOBILE_EGGBERT_CNA_ROOT}/modules/runtime/include/CNA/Misc.hpp")
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

# Link only the CNA modules whose public APIs the game uses. Their PUBLIC dependency closure brings
# in graphics, input, audio, content, storage and the selected renderer; Net and CNAEXT stay out of
# the executable. CNA's module graph also declares its static-library cycles, so consumers no
# longer need renderer-specific linker groups.
target_link_libraries(${_game_target} PRIVATE
    CNA::Runtime
    CNA::Devices
    CNA::GamerServices
    SharpRuntime::IO.IsolatedStorage
)

if(ANDROID)
    # Entrypoint.hpp includes SDL_main.h on Android, so SDL's headers are a direct compile-time
    # dependency here in addition to the transitive native-library dependency from CNA.
    find_package(SDL3 REQUIRED CONFIG)
    target_link_libraries(${_game_target} PRIVATE SDL3::SDL3 android log)
    if(TARGET SDL3::SDL3main)
        target_link_libraries(${_game_target} PRIVATE SDL3::SDL3main)
    endif()
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
    )
    if(CNA_GRAPHICS_RENDERER STREQUAL "WEBGL1")
        target_link_options(${_game_target} PRIVATE
            "-sMIN_WEBGL_VERSION=1"
            "-sMAX_WEBGL_VERSION=1"
        )
    elseif(CNA_GRAPHICS_RENDERER STREQUAL "WEBGL2"
            OR CNA_GRAPHICS_RENDERER STREQUAL "SDL_RENDERER")
        target_link_options(${_game_target} PRIVATE
            "-sMIN_WEBGL_VERSION=2"
            "-sMAX_WEBGL_VERSION=2"
        )
    endif()
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
    if(CNA_GRAPHICS_RENDERER STREQUAL "WEBGPU" AND CNA_WEBGPU_RUNTIME_LIBRARY)
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
