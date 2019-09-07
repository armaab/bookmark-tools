cmake_minimum_required (VERSION 3.12)

project ("pdfmark")

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(WIN32)
    set(GS_LOCATION "${CMAKE_SOURCE_DIR}/lib/ghostscript")
endif()

set(GS_VERBOSE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(BINARY_INSTALL_DIR bin)

# Include sub-projects.
add_subdirectory ("core")
add_subdirectory ("cmd")

if(ENABLE_QT)
    add_subdirectory ("qt")
endif()
