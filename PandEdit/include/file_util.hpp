//  ===== Date Created: 28 April, 2020 ===== 

#if !defined(FILE_UTIL_HPP)
#define FILE_UTIL_HPP

#include <string>

std::string readFile(const char* filename, bool createIfNotExists = false);
std::string getFilenameFromPath(const std::string& path);
std::string getPathOnly(const std::string& path);

#endif
