find_path(GS_INCLUDE_DIR
    NAMES
        ghostscript/gserrors.h ghostscript/iapi.h
    HINTS
        "${GS_LOCATION}"
    PATH_SUFFIXES
        include
)

if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    find_library(GS_LIBRARY_LIB
        NAMES
            gs gsdll64
        HINTS
            "${GS_LOCATION}"
        PATH_SUFFIXES
            lib bin
    )
endif()

set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll" ".so")
find_library(GS_LIBRARY_SHARED
    NAMES
        gs gsdll64
    HINTS
        "${GS_LOCATION}"
    PATH_SUFFIXES
        lib bin
)

if(GS_VERBOSE)
    message(STATUS "FindGS: GS_INCLUDE_DIR: ${GS_INCLUDE_DIR}")
    message(STATUS "FindGS: GS_LIBRARY_LIB: ${GS_LIBRARY_LIB}")
    message(STATUS "FindGS: GS_LIBRARY_SHARED: ${GS_LIBRARY_SHARED}")
endif()

include(FindPackageHandleStandardArgs)

if(WIN32)
    find_package_handle_standard_args(GS
        DEFAULT_MSG
        GS_INCLUDE_DIR
        GS_LIBRARY_LIB
        GS_LIBRARY_SHARED
    )

    add_library(GS::gs SHARED IMPORTED)
    set_target_properties(GS::gs
        PROPERTIES
            IMPORTED_LOCATION "${GS_LIBRARY_SHARED}"
            IMPORTED_IMPLIB "${GS_LIBRARY_LIB}"
            INTERFACE_INCLUDE_DIRECTORIES "${GS_INCLUDE_DIR}"
    )
else()
    find_package_handle_standard_args(GS
        DEFAULT_MSG
        GS_INCLUDE_DIR
        GS_LIBRARY_SHARED
    )

    add_library(GS::gs UNKNOWN IMPORTED)
    set_target_properties(GS::gs
        PROPERTIES
            IMPORTED_LOCATION "${GS_LIBRARY_SHARED}"
            INTERFACE_INCLUDE_DIRECTORIES "${GS_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(GS_INCLUDE_DIR GS_LIBRARY_SHARED GS_LIBRARY_LIB)
