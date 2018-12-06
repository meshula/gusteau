
# author: Nick Porcino
# license: MIT

include(FindPackageHandleStandardArgs)

if(EXISTS "$ENV{USD_ROOT}")
    set(USD_ROOT $ENV{USD_ROOT})
endif()

find_package(Alembic)
find_package(Embree)
find_package(Houdini)
find_package(Katana)
find_package(Maya)

find_path(USD_INCLUDE_DIR pxr/pxr.h
    PATHS ${USD_LOCATION}
          ${USD_ROOT}
          /usr
          /usr/local
          /sw
          /opt/local

    PATH_SUFFIXES
        /include

        NO_DEFAULT_PATH
        NO_CMAKE_ENVIRONMENT_PATH
        NO_CMAKE_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        NO_CMAKE_SYSTEM_PATH
        NO_CMAKE_FIND_ROOT_PATH
    DOC "USD include directory")


if(WIN32)
    set(LIB_EXT "lib")
    set(DYLIB_EXT "dll")
elseif(APPLE)
    set(LIB_EXT "a")
    set(DYLIB_EXT "dylib")
else()
    set(LIB_EXT "a")
    set(DYLIB_EXT "so")
endif()

find_path(USD_LIBRARY_DIR libusd.${DYLIB_EXT}
    PATHS ${USD_INCLUDE_DIR}/../lib
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_CMAKE_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    NO_CMAKE_SYSTEM_PATH
    NO_CMAKE_FIND_ROOT_PATH
    DOC "USD Libraries directory")

find_file(USD_GENSCHEMA
    names usdGenSchema
    PATHS ${USD_INCLUDE_DIR}/../bin
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_CMAKE_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    NO_CMAKE_SYSTEM_PATH
    NO_CMAKE_FIND_ROOT_PATH
    DOC "USD Gen schema application")

find_path(USD_BIN_DIR usdview
    PATHS ${USD_INCLUDE_DIR}/../bin
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_CMAKE_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    NO_CMAKE_SYSTEM_PATH
    NO_CMAKE_FIND_ROOT_PATH
    DOC "USD Bin directory")

if(Katana_FOUND)
    find_path(USD_KATANA_INCLUDE_DIR usdKatana/api.h
        PATHS ${USD_INCLUDE_DIR}/../third_party/katana/include ${KATANA_INCLUDE_DIR}
        NO_DEFAULT_PATH
        NO_CMAKE_ENVIRONMENT_PATH
        NO_CMAKE_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        NO_CMAKE_SYSTEM_PATH
        NO_CMAKE_FIND_ROOT_PATH
        DOC "USD Katana Include directory")

    find_path(USD_KATANA_LIBRARY_DIR libusdKatana.${DYLIB_EXT}
        PATHS ${USD_INCLUDE_DIR}/../third_party/katana/lib ${KATANA_LIBRARY_DIR}
        NO_DEFAULT_PATH
        NO_CMAKE_ENVIRONMENT_PATH
        NO_CMAKE_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        NO_CMAKE_SYSTEM_PATH
        NO_CMAKE_FIND_ROOT_PATH
        DOC "USD Katana Library directory")

    if(USD_KATANA_LIBRARY_DIR)
        mark_as_advanced(USD_KATANA_LIBRARY_DIR USD_KATANA_INCLUDE_DIR)
    endif()
endif()

if(Maya_FOUND)
    find_path(USD_MAYA_INCLUDE_DIR usdMaya/api.h
        PATHS ${USD_INCLUDE_DIR}/../third_party/maya/include ${MAYA_INCLUDE_DIR}
        NO_DEFAULT_PATH
        NO_CMAKE_ENVIRONMENT_PATH
        NO_CMAKE_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        NO_CMAKE_SYSTEM_PATH
        NO_CMAKE_FIND_ROOT_PATH
        DOC "USD Maya Include directory")

    find_path(USD_MAYA_LIBRARY_DIR libusdMaya.${DYLIB_EXT}
        PATHS ${USD_INCLUDE_DIR}/../third_party/maya/lib ${MAYA_LIBRARY_DIR}
        NO_DEFAULT_PATH
        NO_CMAKE_ENVIRONMENT_PATH
        NO_CMAKE_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        NO_CMAKE_SYSTEM_PATH
        NO_CMAKE_FIND_ROOT_PATH
        DOC "USD Maya Library directory")

    if(USD_MAYA_LIBRARY_DIR)
        mark_as_advanced(USD_MAYA_LIBRARY_DIR USD_MAYA_INCLUDE_DIR)
    endif()
endif()

if (Houdini_FOUND)
endif()

set(USD_LIB_NAMES
    arch gf js plug tf tracelite vt work
    cameraUtil garch glf hd hdSt hdx hf pxOsd hdStream
    ar kind pcp sdf usd usdGeom usdHydra usdLux usdRi usdShade usdSkel usdUI usdUtils
    usdImaging usdImagingGL
)

if(Embree_FOUND)
    set(USD_LIB_NAMES ${USD_LIB_NAMES} hdEmbree)
endif()

if(Alembic_FOUND)
    set(USD_LIB_NAMES ${USD_LIB_NAMES} usdAbc)
endif()

foreach(_lib ${USD_LIB_NAMES})
    find_library(USD_${_lib}_LIB_RELEASE ${_lib}
        HINTS ${USD_INCLUDE_DIR}/..

        PATHS
            ${USD_LOCATION}
            ${USD_ROOT}
            /usr
            /usr/local
            /sw
            /opt/local

        PATH_SUFFIXES
            /lib

        NO_DEFAULT_PATH
        NO_CMAKE_ENVIRONMENT_PATH
        NO_CMAKE_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        NO_CMAKE_SYSTEM_PATH
        NO_CMAKE_FIND_ROOT_PATH

        DOC "USD library ${LIB}")

        if(USD_${_lib}_LIB_RELEASE)
            list(APPEND USD_LIBRARIES "${USD_${_lib}_LIB_RELEASE}")
            set(USD_${_lib}_FOUND TRUE)
            set(USD_${_lib}_LIBRARY "${USD_${_lib}_LIB_RELEASE}")
            list(APPEND USD_LIBRARIES "${USD_${_lib}_LIBRARY}")
            mark_as_advanced(USD_${_lib}_LIB_RELEASE)
        else()
            set(USD_${_lib}_FOUND FALSE)
        endif()

endforeach()

if (WIN32 OR APPLE)
    set(USD_DEFINES TF_NO_GNU_EXT)
endif()

find_package_handle_standard_args(USD
    REQUIRED_VARS
        USD_INCLUDE_DIR
        USD_LIBRARY_DIR
        USD_LIBRARIES
        USD_DEFINES)

