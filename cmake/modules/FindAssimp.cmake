
# find Assimp

# source: https://github.com/cginternals/cmake-init/blob/master/cmake/FindGLFW.cmake
# license: MIT

# Assimp_FOUND
# Assimp_INCLUDE_DIR
# Assimp_LIBRARY_RELEASE
# Assimp_LIBRARY_DEBUG
# Assimp_LIBRARIES
# Assimp_BINARY (win32 only)

include(FindPackageHandleStandardArgs)

find_path(Assimp_INCLUDE_DIR assimp/Importer.hpp

    PATHS
    ${ASSIMP_LOCATION}
    $ENV{ASSIMP_DIR}
    $ENV{PROGRAMFILES}/Assimp
    /usr
    /usr/local
    /sw
    /opt/local

    PATH_SUFFIXES
    /include

    DOC "The directory where assimp/Importer.hpp etc. resides")

if(MSVC AND X64)
    set(Assimp_PF "64")
else()
    set(Assimp_PF "86")
endif()

if(MSVC)
    set(ASSIMP_NAMES assimp-vc140-mt assimp)
else()
    set(ASSIMP_NAMES assimp)
endif()

find_library(Assimp_LIBRARY_RELEASE NAMES ${ASSIMP_NAMES}

    HINTS
    ${Assimp_INCLUDE_DIR}/..

    PATHS
    ${ASSIMP_LOCATION}
    $ENV{ASSIMP_DIR}
    /usr
    /usr/local
    /sw
    /opt/local

    PATH_SUFFIXES
    /lib
    /lib${Assimp_PF}
    /build/code
    /build-debug/code

    DOC "The Assimp library (release)")

if(MSVC)
    set(ASSIMP_DEBUG_NAMES assimp-vc140-mtd assimpd)
else()
    set(ASSIMP_DEBUG_NAMES assimpd)
endif()

find_library(Assimp_LIBRARY_DEBUG NAMES ${ASSIMP_DEBUG_NAMES}

    HINTS
    ${Assimp_INCLUDE_DIR}/..

    PATHS
    ${ASSIMP_LOCATION}
    $ENV{ASSIMP_DIR}
    /usr
    /usr/local
    /sw
    /opt/local

    PATH_SUFFIXES
    /lib
    /lib${ASSIMP_PF}
    /build/code
    /build-debug/code

    DOC "The Assimp library (debug)")

set(Assimp_LIBRARIES "")
if(Assimp_LIBRARY_RELEASE AND Assimp_LIBRARY_DEBUG)
    set(Assimp_LIBRARIES
        optimized   ${Assimp_LIBRARY_RELEASE}
        debug       ${Assimp_LIBRARY_DEBUG})
elseif(Assimp_LIBRARY_RELEASE)
    set(Assimp_LIBRARIES ${Assimp_LIBRARY_RELEASE})
elseif(Assimp_LIBRARY_DEBUG)
    set(Assimp_LIBRARIES ${Assimp_LIBRARY_DEBUG})
endif()

if(WIN32)
    set(ASSIMP_DLL_NAMES assimp-vc140-mt.dll assimp.dll "assimp${Assimp_PF}.dll")
    find_file(Assimp_BINARY_RELEASE NAMES ${ASSIMP_DLL_NAMES}

        HINTS
        ${Assimp_INCLUDE_DIR}/..

        PATHS
        ${ASSIMP_LOCATION}
        $ENV{Assimp_DIR}

        PATH_SUFFIXES
        /bin
        /bin${Assimp_PF}

        DOC "The Assimp binary")

    set(ASSIMP_DLL_DEBUG_NAMES assimp-vc140-mtd.dll assimpd.dll "assimp${Assimp_PF}d.dll")
    find_file(Assimp_BINARY_DEBUG NAMES ${ASSIMP_DLL_DEBUG_NAMES}

        HINTS
        ${Assimp_INCLUDE_DIR}/..

        PATHS
        ${ASSIMP_LOCATION}
        $ENV{Assimp_DIR}

        PATH_SUFFIXES
        /bin
        /bin${Assimp_PF}

        DOC "The Assimp binary")

endif()

find_package_handle_standard_args(ASSIMP DEFAULT_MSG Assimp_LIBRARIES Assimp_INCLUDE_DIR)
mark_as_advanced(Assimp_FOUND Assimp_INCLUDE_DIR Assimp_LIBRARIES)
