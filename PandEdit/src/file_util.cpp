//  ===== Date Created: 28 April, 2020 ===== 

#include <fstream>
#include <stdio.h>

#include "file_util.hpp"

std::string readFile(const char* filename, bool createIfNotExists)
{
	if (createIfNotExists)
	{
		// ifstream will fail if the file doesn't exist, but this will not
		std::ofstream file(filename);
	}
	
	std::ifstream file(filename);

	if (!file)
	{
		printf("Error: Failed to read file '%s'.\n", filename);
		return "";
	}

	file.seekg(0, std::ios::end);
	std::size_t size = file.tellg();
	file.seekg(0);

	std::string buffer(size, ' ');
	file.read(&buffer[0], size);

	return buffer;
}