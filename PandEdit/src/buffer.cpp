//  ===== Date Created: 15 April, 2020 =====

#include <fstream>

#include "buffer.hpp"
#include "frame.hpp"
#include "file_util.hpp"

Buffer::Buffer(BufferType type, std::string name, std::string path)
	: type(type), name(name), path(path)
{
	if (path == "")
	{
		// Makes sure there's at least one line in the buffer
		data.emplace_back();
	}
	else
	{
		std::string fileContents = readFile(path.c_str(), true);

		std::string::size_type pos = 0;
		std::string::size_type previous = 0;

		while ((pos = fileContents.find("\n", previous)) != std::string::npos)
		{
			data.emplace_back(std::move(fileContents.substr(previous, pos - previous)));
			previous = pos + 1;
		}

		// Last one
		data.emplace_back(std::move(fileContents.substr(previous)));
	}
	
	buffersMap.insert({ name, this });
}

Buffer::~Buffer()
{
	buffersMap.erase(name);

	for (Frame& frame : *Frame::allFrames)
	{
		if (this == frame.currentBuffer)
		{
			// TODO(fkp): Go to previous
			frame.currentBuffer = get("*scratch*");
		}
	}
}

Buffer::Buffer(Buffer&& other)
	: type(other.type), name(std::move(other.name)), data(std::move(other.data)),
	  lastPoint(other.lastPoint)
{
	buffersMap[name] = this;
	other.name = "";
}

Buffer& Buffer::operator=(Buffer&& other)
{
	if (this != &other)
	{
		type = other.type;
		name = std::move(other.name);
		data = std::move(other.data);

		lastPoint = other.lastPoint;

		buffersMap[name] = this;
		other.name = "";
	}

	return *this;
}

Buffer* Buffer::get(const std::string& name)
{
	auto result = buffersMap.find(name);

	if (result != buffersMap.end())
	{
		return result->second;
	}
	else
	{
		return nullptr;
	}
}

Buffer* Buffer::getFromFilePath(const std::string& path)
{
	// TODO(fkp): Iterating an unordered_map is really bad.
	for (std::pair<const std::string, Buffer*>& pair : buffersMap)
	{
		if (pair.second->path == path)
		{
			return pair.second;
		}
	}

	return nullptr;
}

void Buffer::saveToFile()
{
	// TODO(fkp): Check if changes need to be saved
	if (path == "")
	{
		printf("Error: Cannot save non-file-visiting buffer.\n");
		return;
	}

	std::ofstream file(path, std::ios::trunc);

	if (!file)
	{
		printf("Error: Failed to open file '%s' for saving to.\n", path.c_str());
		return;
	}

	for (const std::string& line : data)
	{
		file << line << '\n';
	}

	printf("Info: Saved buffer to file '%s'.\n", path.c_str());
}
