# ===== Date Created: 14 April, 2020 ===== 
cmake_minimum_required(VERSION 3.15)

project(PandEdit)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Projects
add_subdirectory(${CMAKE_SOURCE_DIR}/PandEdit)
