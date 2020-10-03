# Function to add an executable target
# e.g. for each chapter
function(add_exec_target CHAPTER)
	add_executable(Gusteau-${CHAPTER} "")

	target_link_libraries(Gusteau-${CHAPTER} "glfw" "${GLFW_LIBRARIES}")
	target_include_directories(Gusteau-${CHAPTER} PRIVATE "${GLFW_DIR}/include")
	target_compile_definitions(Gusteau-${CHAPTER} PRIVATE "GLFW_INCLUDE_NONE")

	target_sources(Gusteau-${CHAPTER} PRIVATE 
		src/${CHAPTER}.cpp 
		src/blackboard.h
		src/ConcurrentQueue.h
		src/csp.h
		src/journal.h
		src/TypedData.h
		src/LabText.h
		third-party/imgui/imgui.cpp 
		third-party/imgui/imgui.h
		third-party/imgui/imgui_draw.cpp 
		third-party/imgui/imgui_internal.h
		third-party/imgui/imgui_widgets.cpp
		third-party/imgui/examples/imgui_impl_glfw.cpp
		third-party/imgui/examples/imgui_impl_glfw.h
		third-party/imgui/examples/imgui_impl_opengl3.cpp
		third-party/imgui/examples/imgui_impl_opengl3.h
		third-party/glew/glew.c)

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
		${GUSTEAU_ROOT}/third-party/glew/include
		${GLFW_INCLUDE_DIR}
		${GUSTEAU_ROOT}/src
		${GUSTEAU_ROOT}/third-party/imgui
		${GUSTEAU_ROOT}/third-party)

	target_link_libraries(Gusteau-${CHAPTER} debug
		${OPENGL_LIBRARIES} ${glfw_LIBRARIES} ${PTHREAD_LIB})
	target_link_libraries(Gusteau-${CHAPTER} optimized
		${OPENGL_LIBRARIES} ${glfw_LIBRARIES} ${PTHREAD_LIB})

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
endfunction()
