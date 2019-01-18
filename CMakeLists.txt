
cmake_minimum_required(VERSION 3.10)

if (NOT CHAPTER)
    set(CHAPTER chapter1)
endif()

project(Gusteau-${CHAPTER})

set(GUSTEAU_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# override cmake install prefix if it's not set,
# per: http://public.kitware.com/pipermail/cmake/2010-December/041135.html
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX ${LOCAL_ROOT} CACHE PATH "Install set to local" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CMAKE_BUILD_TYPE Release)

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

add_executable(Gusteau-${CHAPTER} "")

include(CXXDefaults)
add_definitions(${_PXR_CXX_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${_PXR_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")

find_package(OpenGL REQUIRED)
find_package(GLFW REQUIRED)

target_sources(Gusteau-${CHAPTER} PRIVATE 
    src/${CHAPTER}.cpp 
    src/imgui/imgui.cpp 
    src/imgui/imgui.h
    src/imgui/imgui_draw.cpp 
    src/imgui/imgui_internal.h
    src/imgui/imgui_widgets.cpp
    src/imgui/examples/imgui_impl_glfw.cpp
    src/imgui/examples/imgui_impl_glfw.h
    src/imgui/examples/imgui_impl_opengl3.cpp
    src/imgui/examples/imgui_impl_opengl3.h
    src/glew/glew.c)

target_compile_definitions(Gusteau-${CHAPTER} PUBLIC ${USD_DEFINES})
target_compile_definitions(Gusteau-${CHAPTER} PUBLIC GUSTEAU_${CHAPTER})
target_compile_definitions(Gusteau-${CHAPTER} PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)

if (WIN32)
    target_compile_definitions(Gusteau-${CHAPTER} PUBLIC PLATFORM_WINDOWS)
    target_compile_definitions(Gusteau-${CHAPTER} PUBLIC __TBB_NO_IMPLICIT_LINKAGE=1)
    target_compile_definitions(Gusteau-${CHAPTER} PUBLIC __TBBMALLOC_NO_IMPLICIT_LINKAGE=1)
elseif (APPLE)
    target_compile_definitions(Gusteau-${CHAPTER} PUBLIC PLATFORM_DARWIN)
    target_compile_definitions(Gusteau-${CHAPTER} PUBLIC PLATFORM_MACOS)
else()
    target_compile_definitions(Gusteau-${CHAPTER} PUBLIC PLATFORM_LINUX)
endif()

target_compile_definitions(Gusteau-${CHAPTER} PUBLIC GLEW_STATIC)

target_include_directories(Gusteau-${CHAPTER} PRIVATE
    ${GUSTEAU_ROOT}/src/glew/include
    ${GLFW_INCLUDE_DIR}
    ${GUSTEAU_ROOT}/src
    ${GUSTEAU_ROOT}/src/imgui)

if (UNIX)
    set(PTHREAD_LIB pthread)
endif()

target_link_libraries(Gusteau-${CHAPTER} debug
    ${OPENGL_LIBRARIES} ${GLFW_LIBRARY_RELEASE} ${PTHREAD_LIB})
target_link_libraries(Gusteau-${CHAPTER} optimized
    ${OPENGL_LIBRARIES} ${GLFW_LIBRARY_RELEASE} ${PTHREAD_LIB})

if (MSVC_IDE)
    # hack to get around the "Debug" and "Release" directories cmake tries to add on Windows
    #set_target_properties(Gusteau-${CHAPTER} PROPERTIES PREFIX "../")
    #set_target_properties(Gusteau-${CHAPTER} PROPERTIES IMPORT_PREFIX "../")
endif()

set_target_properties(Gusteau-${CHAPTER}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

install (TARGETS Gusteau-${CHAPTER}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

install (TARGETS Gusteau-${CHAPTER} DESTINATION bin)

file(GLOB src "src/*")
source_group(src FILES ${src})
