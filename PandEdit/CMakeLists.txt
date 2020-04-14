# ===== Date Created: 14 April, 2020 ===== 

project(PandEdit_Core)

# Files to use
set(HEADERS)
set(SOURCES main.cpp)

# Prepends directories to the files
list(TRANSFORM HEADERS PREPEND include/)
list(TRANSFORM SOURCES PREPEND src/)

add_executable(PandEdit_Core ${SOURCES} ${HEADERS})
set_target_properties(PandEdit_Core PROPERTIES OUTPUT_NAME "PandEdit")
target_include_directories(PandEdit_Core PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(PandEdit_Core PRIVATE glad)
