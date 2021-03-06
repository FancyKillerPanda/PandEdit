# ===== Date Created: 14 April, 2020 ===== 

project(PandEdit_Core)

# Copy resources to output directory
set(RESOURCE_FILES arial.ttf consola.ttf shape.vert shape.frag texture.vert texture.frag)
add_custom_target(ResourceFileCopyTarget)

foreach (file IN LISTS RESOURCE_FILES)
	if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res/${file}")
		add_custom_command(
			TARGET ResourceFileCopyTarget POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E echo "Resource file '${file}' is missing."
		)
	else ()
		add_custom_command(
			TARGET ResourceFileCopyTarget POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/res/${file}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/res/${file}"
			DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/res/${file}"
		)
	endif ()
endforeach ()

# Files to use
set(HEADERS
	common.hpp
	window.hpp
	shader.hpp
	matrix.hpp
	font.hpp
	renderer.hpp
	frame.hpp
	buffer.hpp
	commands.hpp
	key.hpp
	key_combo.hpp
	keymap.hpp
	default_key_bindings.hpp
	point.hpp
	file_util.hpp
	undo.hpp
	timer.hpp
	lexer.hpp
	token.hpp
	colour.hpp
	line_lex_state.hpp
	project.hpp
)
set(SOURCES
	main.cpp
	window.cpp
	shader.cpp
	matrix.cpp
	font.cpp
	renderer.cpp
	frame.cpp
	buffer.cpp
	commands.cpp
	key.cpp
	key_combo.cpp
	keymap.cpp
	point.cpp
	file_util.cpp
	undo.cpp
	timer.cpp
	lexer.cpp
	token.cpp
	colour.cpp
	line_lex_state.cpp
	project.cpp
)

# Prepends directories to the files
list(TRANSFORM HEADERS PREPEND include/)
list(TRANSFORM SOURCES PREPEND src/)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(PandEdit_Core ${SOURCES} ${HEADERS})
set_target_properties(PandEdit_Core PROPERTIES OUTPUT_NAME "PandEdit")
target_include_directories(PandEdit_Core PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(PandEdit_Core PRIVATE glad)
target_link_libraries(PandEdit_Core PRIVATE freetype)
add_dependencies(PandEdit_Core ResourceFileCopyTarget)

if (WIN32)
	target_link_libraries(PandEdit_Core PRIVATE opengl32.lib gdi32.lib)
endif ()

IF (MSVC)
	target_compile_options(PandEdit_Core PRIVATE /WX)
ELSE ()
	target_compile_options(PandEdit_Core PRIVATE -Werror)
ENDIF ()
