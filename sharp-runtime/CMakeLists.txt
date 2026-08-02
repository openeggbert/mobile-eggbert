cmake_minimum_required(VERSION 3.20)

project(SHARP_RUNTIME LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SHARP_RUNTIME_BUILD_TESTS "Build sharp-runtime tests" ON)
option(SHARP_RUNTIME_BUILD_BENCHMARKS "Build sharp-runtime standalone micro-benchmarks" OFF)

file(GLOB_RECURSE SHARP_RUNTIME_SOURCES CONFIGURE_DEPENDS
        "src/*.cpp"
)

add_library(SHARP_RUNTIME STATIC
        ${SHARP_RUNTIME_SOURCES}
)

target_include_directories(SHARP_RUNTIME PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# On Android, StoragePaths.cpp uses SDL_GetPrefPath / SDL_GetAndroidInternalStoragePath
# which require SDL2 headers and library.  SDL2 is provided by the parent project
# (CNA) via cna_configure_vendored_sdl() before this subdirectory is added.

if(MSVC)
    target_compile_options(SHARP_RUNTIME PRIVATE /W4 /WX)
else()
    target_compile_options(SHARP_RUNTIME PRIVATE -Wall -Wextra -Werror)
    # GCC diagnoses format truncation; Clang (including Android NDK Clang)
    # does not recognise this warning-control option and treats it as an error
    # under -Werror.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(SHARP_RUNTIME PRIVATE -Wno-format-truncation)
    endif()
endif()

if(ANDROID)
    # The runtime intentionally retains std::experimental::filesystem for its
    # C++14-compatible public API.  libc++ deprecates that namespace in C++17.
    target_compile_definitions(SHARP_RUNTIME PRIVATE
        _LIBCPP_NO_EXPERIMENTAL_DEPRECATION_WARNING_FILESYSTEM)
endif()

if(WIN32)
    target_link_libraries(SHARP_RUNTIME PRIVATE ws2_32)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT ANDROID)
    # Desktop libstdc++ needs this archive for the Filesystem TS. Android's
    # libc++ provides it directly and has no libstdc++fs archive.
    target_link_libraries(SHARP_RUNTIME PRIVATE stdc++fs)
endif()

if(ANDROID AND TARGET SDL2::SDL2)
    target_link_libraries(SHARP_RUNTIME PRIVATE SDL2::SDL2)
elseif(ANDROID AND TARGET SDL2::SDL2-static)
    target_link_libraries(SHARP_RUNTIME PRIVATE SDL2::SDL2-static)
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
