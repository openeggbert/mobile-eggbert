cmake_minimum_required(VERSION 3.21)

foreach(_runtime_dll IN LISTS RUNTIME_DLLS)
    if(NOT _runtime_dll STREQUAL "")
        get_filename_component(_runtime_name "${_runtime_dll}" NAME)
        file(COPY_FILE "${_runtime_dll}" "${DESTINATION}/${_runtime_name}" ONLY_IF_DIFFERENT)
    endif()
endforeach()
