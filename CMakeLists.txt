
cmake_minimum_required(VERSION 3.10)

set(PREFIX_PROJECT_NAME "Gusteau")

# all available chapters
set(CHAPTER_LIST "chapter1;chapter2;chapter3" CACHE INTERNAL "")

# chapter now used to generate a specific chapter solution file only
# if not specify then generate a solution file contains all chapters
if (NOT CHAPTER)
    set(ALL_CHAPTERS TRUE CACHE INTERNAL "")
endif()

if (CHAPTER)
    project(${PREFIX_PROJECT_NAME}-${CHAPTER})
else()
    project(${PREFIX_PROJECT_NAME})
endif()

set(GUSTEAU_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/defaults
                      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
                      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/macros)

include(Utilities)

# Turn on folder usage
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

find_package(OpenGL REQUIRED)

# glfw
set(GLFW_DIR "third-party/glfw")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory("${GLFW_DIR}")

# chapter setup
include(Commons)
if (CHAPTER)
    message(STATUS "Generate solution for '${CHAPTER}'")
    add_exec_target(${CHAPTER})
else()
    message(STATUS "Generate solutions for all chapters")
    foreach(chapter ${CHAPTER_LIST})
        add_exec_target(${chapter})
        message(STATUS "Generated for ${chapter}")
    endforeach()
endif()

include(CXXDefaults)
add_definitions(${_PXR_CXX_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${_PXR_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")

if (UNIX)
    set(PTHREAD_LIB pthread)
endif()

file(GLOB src "src/*")
source_group(src FILES ${src})
