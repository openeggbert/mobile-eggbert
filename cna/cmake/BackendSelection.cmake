# --- Graphics Backend Selection ---
# mobile-eggbert vendored copy: only SDL_RENDERER, HEADLESS, and SOFTWARE are kept (see
# plan_lite.md Phase 3). The other 11 backends this file used to support (EASYGL, BGFX, VULKAN,
# WEBGPU, D3D11, D3D12, CANVAS, ASCII, DX3, D3D9, SDL_GPU) were removed along with their source
# directories under src/CNA/Internal/Backends and include/CNA/Internal/Backends.
set(CNA_GRAPHICS_BACKEND "SDL_RENDERER" CACHE STRING "Graphics backend to use (SDL_RENDERER, HEADLESS, or SOFTWARE)")
set_property(CACHE CNA_GRAPHICS_BACKEND PROPERTY STRINGS "SDL_RENDERER" "HEADLESS" "SOFTWARE")

option(CNA_BACKEND_SDL_RENDERER "Enable SDL_Renderer graphics backend" OFF)
option(CNA_BACKEND_HEADLESS "Enable Headless (no GPU/window) graphics backend" OFF)
option(CNA_BACKEND_SOFTWARE "Enable Software (CPU rasterizer) graphics backend" OFF)

set(_cna_explicit_backend_selection OFF)
if(CNA_BACKEND_SDL_RENDERER OR CNA_BACKEND_HEADLESS OR CNA_BACKEND_SOFTWARE)
    set(_cna_explicit_backend_selection ON)
endif()

if(_cna_explicit_backend_selection)
    set(_cna_enabled_backends)
    if(CNA_BACKEND_SDL_RENDERER)
        list(APPEND _cna_enabled_backends "SDL_RENDERER")
    endif()
    if(CNA_BACKEND_HEADLESS)
        list(APPEND _cna_enabled_backends "HEADLESS")
    endif()
    if(CNA_BACKEND_SOFTWARE)
        list(APPEND _cna_enabled_backends "SOFTWARE")
    endif()

    list(LENGTH _cna_enabled_backends _cna_enabled_backends_count)
    if(NOT _cna_enabled_backends_count EQUAL 1)
        message(FATAL_ERROR "CNA: Exactly one backend option must be ON when using CNA_BACKEND_* options.")
    endif()

    list(GET _cna_enabled_backends 0 CNA_GRAPHICS_BACKEND)
endif()

if(CNA_GRAPHICS_BACKEND STREQUAL "SDL_RENDERER")
    message(STATUS "CNA: Using SDL_RENDERER graphics backend")
    set(BACKEND_DIR "src/CNA/Internal/Backends/SdlRenderer")
    set(BACKEND_TARGET "cna_backend_graphics_sdl_renderer")
    add_compile_definitions(CNA_BACKEND_SDL_RENDERER)
    set(CNA_BACKEND_DEFINE "CNA_BACKEND_SDL_RENDERER")
elseif(CNA_GRAPHICS_BACKEND STREQUAL "HEADLESS")
    message(STATUS "CNA: Using HEADLESS (no GPU/window) graphics backend")
    set(BACKEND_DIR "src/CNA/Internal/Backends/Headless")
    set(BACKEND_TARGET "cna_backend_graphics_headless")
    add_compile_definitions(CNA_BACKEND_HEADLESS)
    set(CNA_BACKEND_DEFINE "CNA_BACKEND_HEADLESS")
elseif(CNA_GRAPHICS_BACKEND STREQUAL "SOFTWARE")
    message(STATUS "CNA: Using SOFTWARE (CPU rasterizer) graphics backend")
    set(BACKEND_DIR "src/CNA/Internal/Backends/Software")
    set(BACKEND_TARGET "cna_backend_graphics_software")
    add_compile_definitions(CNA_BACKEND_SOFTWARE)
    set(CNA_BACKEND_DEFINE "CNA_BACKEND_SOFTWARE")
else()
    message(FATAL_ERROR "CNA: Unknown graphics backend: ${CNA_GRAPHICS_BACKEND}")
endif()
