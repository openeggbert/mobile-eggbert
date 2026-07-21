cmake_minimum_required(VERSION 3.20)

project(SHARP_RUNTIME LANGUAGES CXX C)

find_package(ZLIB REQUIRED)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SHARP_RUNTIME_BUILD_TESTS "Build sharp-runtime tests" ON)
option(SHARP_RUNTIME_BUILD_BENCHMARKS "Build sharp-runtime standalone micro-benchmarks" OFF)

file(GLOB_RECURSE SHARP_RUNTIME_SOURCES CONFIGURE_DEPENDS
        "src/*.cpp"
)

# Vendored tinyxml2 (two-file library, MIT licence)
add_library(tinyxml2 STATIC vendor/tinyxml2/tinyxml2.cpp)
target_include_directories(tinyxml2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyxml2)

# Vendored miniz (multi-file library, MIT/public domain)
add_library(miniz STATIC
    vendor/miniz/miniz.c
    vendor/miniz/miniz_tdef.c
    vendor/miniz/miniz_tinfl.c
    vendor/miniz/miniz_zip.c
)
target_include_directories(miniz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/miniz)

add_library(SHARP_RUNTIME STATIC
        ${SHARP_RUNTIME_SOURCES}
)

target_include_directories(SHARP_RUNTIME PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(SHARP_RUNTIME SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

# On Android, StoragePaths.cpp uses SDL_GetPrefPath / SDL_GetAndroidInternalStoragePath
# which require SDL3 headers and library.  SDL3 is provided by the parent project
# (CNA) via cna_configure_vendored_sdl() before this subdirectory is added.
target_link_libraries(SHARP_RUNTIME PRIVATE ZLIB::ZLIB tinyxml2 miniz)

if(MSVC)
    target_compile_options(SHARP_RUNTIME PRIVATE /W4 /WX)
else()
    target_compile_options(SHARP_RUNTIME PRIVATE -Wall -Wextra -Werror -Wno-format-truncation)
endif()

if(WIN32)
    target_link_libraries(SHARP_RUNTIME PRIVATE ws2_32)
endif()

if(ANDROID AND TARGET SDL3::SDL3)
    target_link_libraries(SHARP_RUNTIME PRIVATE SDL3::SDL3)
elseif(ANDROID AND TARGET SDL3::SDL3-static)
    target_link_libraries(SHARP_RUNTIME PRIVATE SDL3::SDL3-static)
endif()

if(SHARP_RUNTIME_BUILD_TESTS)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/googletest/CMakeLists.txt")
        message(FATAL_ERROR
            "vendor/googletest is missing.\n"
            "Run: git submodule update --init --recursive")
    endif()
    add_subdirectory(vendor/googletest)

    enable_testing()

    file(GLOB_RECURSE SHARP_RUNTIME_TEST_SOURCES CONFIGURE_DEPENDS
            "tests/*.cpp"
    )

    add_executable(SharpRuntimeTests
            ${SHARP_RUNTIME_TEST_SOURCES}
    )

    target_link_libraries(SharpRuntimeTests
            PUBLIC
            SHARP_RUNTIME
            PRIVATE
            gtest_main
    )

    if(NOT MSVC)
        target_compile_options(SharpRuntimeTests PRIVATE -Wall -Wextra -Werror -Wno-format-truncation -Wno-unused-result)
    endif()

    include(GoogleTest)
    gtest_discover_tests(SharpRuntimeTests)
endif()

if(SHARP_RUNTIME_BUILD_BENCHMARKS)
    add_executable(SharpRuntimeBench
            bench/StringBenchmark.cpp
    )

    target_link_libraries(SharpRuntimeBench PRIVATE SHARP_RUNTIME)

    if(NOT MSVC)
        target_compile_options(SharpRuntimeBench PRIVATE -Wall -Wextra -Werror -O2)
    else()
        target_compile_options(SharpRuntimeBench PRIVATE /O2)
    endif()
endif()