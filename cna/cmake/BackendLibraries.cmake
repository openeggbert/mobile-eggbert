# Common backend interfaces
add_library(cna_backend_graphics_common INTERFACE)
target_include_directories(cna_backend_graphics_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Backend target
file(GLOB BACKEND_SOURCES "${BACKEND_DIR}/*.cpp")

add_library(${BACKEND_TARGET} STATIC ${BACKEND_SOURCES})
target_link_libraries(${BACKEND_TARGET} PUBLIC cna_backend_graphics_common SHARP_RUNTIME)
target_include_directories(${BACKEND_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(CNA_GRAPHICS_BACKEND STREQUAL "SDL_RENDERER")
    target_link_libraries(${BACKEND_TARGET} PRIVATE SDL3::SDL3)
endif()
