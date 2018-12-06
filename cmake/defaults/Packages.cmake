
link_directories(${USD_ROOT}/lib)

# -- OpenGL
find_package(OpenGL REQUIRED)

set(GLFW_LOCATION ${LOCAL_ROOT})
find_package(GLFW REQUIRED)

set(USD_ROOT ${LOCAL_ROOT})
find_package(USD REQUIRED)
set(USD_LOCATION "${USD_INCLUDE_DIR}/..")

set(GLEW_LOCATION ${USD_LOCATION})
find_package(GLEW REQUIRED)

# USD requires Boost
set(BOOST_ROOT ${USD_ROOT})
set(BOOST_LIBRARYDIR ${USD_ROOT}/lib)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS python)
include_directories(${Boost_INCLUDE_DIR})

# USD requires TBB
set(TBB_ROOT_DIR ${USD_LOCATION})
find_package(TBB REQUIRED)

# USD requires Python
find_package(PythonInterp 2.7 REQUIRED)
find_package(PythonLibs 2.7 REQUIRED)

set(LABCMD_LOCATION ${LOCAL_ROOT})
find_package(LabCmd REQUIRED)

set(LABRENDER_LOCATION ${LOCAL_ROOT})
find_package(LabRender REQUIRED)

# --math
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_library(M_LIB m)
endif()

# --z
if (WIN32)
    find_file(Z_BINARY_RELEASE
        NAMES
            zlib.dll
        HINTS
            "${LOCAL_ROOT}/bin"
        DOC "The z library")
endif()
