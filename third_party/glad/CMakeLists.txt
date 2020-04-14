# ===== Date Created: 14 April, 2020 ===== 

project(glad)

# Files to use
set(HEADERS
	glad/glad.h
	glad/glad_wgl.h
	KHR/khrplatform.h
)
set(SOURCES
	glad.c
	glad_wgl.c
)

# Prepends directories to the files
list(TRANSFORM HEADERS PREPEND include/)
list(TRANSFORM SOURCES PREPEND src/)

# Library
add_library(glad ${SOURCES} ${HEADERS})
target_include_directories(glad PUBLIC include)
