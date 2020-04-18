//  ===== Date Created: 15 April, 2020 ===== 

#if !defined(FRAME_HPP)
#define FRAME_HPP

#include <string>
#include <unordered_map>

#include "buffer.hpp"

class Frame
{
public:
	static Frame* currentFrame;
	static Frame* previousFrame;
	static Frame* minibufferFrame;

	std::string name;
	
	int x;
	int y;
	unsigned int width;
	unsigned int height;

	Buffer* currentBuffer = nullptr;

private:
	static std::unordered_map<std::string, Frame*> framesMap;

public:
	Frame(std::string name, int x, int y, unsigned int width, unsigned int height, Buffer* buffer = nullptr, bool isActive = false);
	Frame(std::string name, int x, int y, unsigned int width, unsigned int height, BufferType type, std::string bufferName, bool isActive = false);
	~Frame();
	Frame(const Frame&) = delete;
	Frame& operator=(const Frame&) = delete;
	Frame(Frame&& other);
	Frame& operator=(Frame&& other);
	
	static Frame* get(const std::string& name);
	
	void makeActive();

private:
	void init(std::string name, int x, int y, unsigned int width, unsigned int height, Buffer* buffer = nullptr, bool isActive = false);
};

#endif
