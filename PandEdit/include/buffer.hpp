//  ===== Date Created: 15 April, 2020 ===== 

#if !defined(BUFFER_HPP)
#define BUFFER_HPP

#include <string>
#include <vector>
#include <unordered_map>

// NOTE(fkp): This is for DWORD, including <windows.h> gives errors
// for some reason.
#include <IntSafe.h>

#include "point.hpp"

class Frame;

enum class BufferType
{
	MiniBuffer,
	Text,
};

class Buffer
{
public:
	static std::unordered_map<std::string, Buffer*> buffersMap;
	
	BufferType type;
	std::string name;
	std::string path;
	
	std::vector<std::string> data;
	
	// The frame will take a copy of this when opened, and the last
	// frame to close this buffer will write its values in.
	Point lastPoint;
	
public:
	Buffer(BufferType type, std::string name, std::string path);
	~Buffer();
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer(Buffer&& other);
	Buffer& operator=(Buffer&& other);
	
	static Buffer* get(const std::string& name);
	static Buffer* getFromFilePath(const std::string& path);
	
	void saveToFile();
};

#endif
