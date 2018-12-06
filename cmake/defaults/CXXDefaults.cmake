
include(CXXHelpers)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

if (CMAKE_COMPILER_IS_GNUCXX)
    include(gccdefaults)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    include(clangdefaults)
elseif(MSVC)
    include(msvcdefaults)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PUBLIC BUILD_OPTLEVEL_OPT)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_definitions(${PROJECT_NAME} PUBLIC BUILD_OPTLEVEL_OPT)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_definitions(${PROJECT_NAME} PUBLIC BUILD_OPTLEVEL_OPT)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PUBLIC BUILD_OPTLEVEL_DEV)
endif()
