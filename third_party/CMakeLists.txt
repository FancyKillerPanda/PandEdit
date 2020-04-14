# ===== Date Created: 14 April, 2020 ===== 

project(ThirdParty)

# Disable FreeType dependencies
set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_PNG TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_BZip2 TRUE)

# Projects
add_subdirectory(glad)
add_subdirectory(freetype-2.10.1)
