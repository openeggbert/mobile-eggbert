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

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CNA_BACKEND_SDL_RENDERER OFF CACHE BOOL "Enable SDL_Renderer backend in CNA")
set(CNA_BACKEND_EASY_GL ON CACHE BOOL "Enable easy-gl backend in CNA")
set(CNA_BACKEND_BGFX OFF CACHE BOOL "Enable bgfx backend in CNA")
set(CNA_BACKEND_VULKAN OFF CACHE BOOL "Enable Vulkan backend in CNA")
set(CNA_GRAPHICS_BACKEND "EASY_GL" CACHE STRING "Selected CNA backend")

add_subdirectory(../../cna CNA)
include_directories(../cna/include CNA)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(WindowsPhoneSpeedyBlupi ${SOURCES}
)

target_include_directories(WindowsPhoneSpeedyBlupi PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)

# CNA and the selected graphics backend static library have circular
# symbol references on Linux (backend uses Color/Rectangle/Matrix from CNA,
# CNA uses backend interfaces). Wrap them in a linker group so GNU ld
# resolves both without depending on link order.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT WIN32)
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
        target_link_libraries(WindowsPhoneSpeedyBlupi PRIVATE
            -Wl,--start-group CNA ${_cna_backend_target} -Wl,--end-group
            SDL3::SDL3)
    else()
        target_link_libraries(WindowsPhoneSpeedyBlupi PRIVATE CNA SDL3::SDL3)
    endif()
else()
    target_link_libraries(WindowsPhoneSpeedyBlupi PRIVATE CNA SDL3::SDL3)
endif()

if(TARGET SDL3::SDL3main)
    target_link_libraries(WindowsPhoneSpeedyBlupi PRIVATE SDL3::SDL3main)
endif()

set(MOBILE_EGGBERT_CONTENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Content")
if(EXISTS "${MOBILE_EGGBERT_CONTENT_DIR}")
    add_custom_command(TARGET WindowsPhoneSpeedyBlupi POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${MOBILE_EGGBERT_CONTENT_DIR}"
            "$<TARGET_FILE_DIR:WindowsPhoneSpeedyBlupi>/Content"
            COMMENT "Copying game content next to executable")
endif()

if(WIN32)
    add_custom_command(TARGET WindowsPhoneSpeedyBlupi POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:WindowsPhoneSpeedyBlupi>
            $<TARGET_FILE_DIR:WindowsPhoneSpeedyBlupi>
            COMMAND_EXPAND_LISTS
            COMMENT "Copying runtime DLL dependencies")
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
