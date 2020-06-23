//  ===== Date Created: 28 April, 2020 ===== 

#include <sstream>
#include <fstream>
#include <stdio.h>

#include "file_util.hpp"

std::string readFile(const char* filename, bool createIfNotExists)
{
	if (createIfNotExists)
	{
		// ifstream will fail if the file doesn't exist, but this will not
		std::ofstream file(filename, std::ios::app);
	}
	
	std::ifstream file(filename);

	if (!file)
	{
		printf("Error: Failed to read file '%s'.\n", filename);
		return "";
	}

	/* NOTE(fkp): This doesn't work because of Windows' \r\n line endings
	file.seekg(0, std::ios::end);
	std::size_t size = file.tellg();
	file.seekg(0);

	std::string buffer(size, ' ');
	file.read(&buffer[0], size);
	*/

	std::string buffer;
	std::string line;

	while (std::getline(file, line))
	{
		buffer += line + '\n';
	}

	return buffer;
}

bool doesFileExist(const char* path)
{
	std::ifstream file(path);
	return (bool) file;
}

std::string getFilenameFromPath(const std::string& path)
{
	std::size_t lastSlashIndex = path.find_last_of("/\\");
	std::size_t startIndex = lastSlashIndex == std::string::npos ? 0 : lastSlashIndex + 1;

	return path.substr(startIndex);
}

std::string getPathOnly(const std::string& path)
{
	std::size_t lastSlashIndex = path.find_last_of("/\\");
	return path.substr(0, lastSlashIndex + 1);
}
