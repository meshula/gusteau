
# Disable warning C4996 regarding fopen(), strcpy(), etc.
_add_define("_CRT_SECURE_NO_WARNINGS")

# Disable warning C4996 regarding unchecked iterators for std::transform,
# std::copy, std::equal, et al.
_add_define("_SCL_SECURE_NO_WARNINGS")

# Make sure WinDef.h does not define min and max macros which
# will conflict with std::min() and std::max().
_add_define("NOMINMAX")

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

if (ALL_CHAPTERS)
    foreach(chapter ${CHAPTER_LIST})
        target_compile_options(${PREFIX_PROJECT_NAME}-${chapter} PRIVATE /arch:AVX /Zi )
    endforeach()
else()
    target_compile_options(${PREFIX_PROJECT_NAME}-${CHAPTER} PRIVATE /arch:AVX /Zi )
endif()
