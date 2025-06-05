cmake_minimum_required(VERSION 3.10)
project(WindowsPhoneSpeedyBlupi)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(../../cna CNA)
include_directories(../cna/include CNA)

#add_subdirectory(../../neo-sdk NEO_SDK)
#include_directories(../neo-sdk/include NEO_SDK)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(WindowsPhoneSpeedyBlupi ${SOURCES}
        ../../cna/src/System/Exception.cpp
)

target_include_directories(WindowsPhoneSpeedyBlupi PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)

target_link_libraries(WindowsPhoneSpeedyBlupi PRIVATE CNA SDL3::SDL3)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
endif()
