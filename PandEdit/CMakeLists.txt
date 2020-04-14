# ===== Date Created: 14 April, 2020 ===== 

project(PandEdit_Core)

# Files to use
set(HEADERS
	window.hpp)
set(SOURCES
	main.cpp
	window.cpp)

# Prepends directories to the files
list(TRANSFORM HEADERS PREPEND include/)
list(TRANSFORM SOURCES PREPEND src/)

add_executable(PandEdit_Core ${SOURCES} ${HEADERS})
set_target_properties(PandEdit_Core PROPERTIES OUTPUT_NAME "PandEdit")
target_include_directories(PandEdit_Core PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(PandEdit_Core PRIVATE glad)
target_link_libraries(PandEdit_Core PRIVATE freetype)

if (WIN32)
	target_link_libraries(PandEdit_Core PRIVATE opengl32.lib gdi32.lib)
endif (WIN32)
