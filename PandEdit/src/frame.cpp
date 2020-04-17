//  ===== Date Created: 15 April, 2020 =====

#include "frame.hpp"

Frame* Frame::currentFrame = nullptr;
Frame* Frame::previousFrame = nullptr;
Frame* Frame::minibufferFrame = nullptr;
std::unordered_map<std::string, Frame*> Frame::framesMap;

Frame::Frame(std::string name, int x, int y, unsigned int width, unsigned int height, Buffer* buffer, BufferType type, bool isActive)
	: name(name), x(x), y(y), width(width), height(height)
{
	framesMap.insert({ name, this });

	if (!buffer)
	{
		currentBuffer = new Buffer { type };
	}

	if (isActive)
	{
		makeActive();
	}

	if (currentBuffer->type == BufferType::MiniBuffer)
	{
		minibufferFrame = this;
	}
}

Frame::~Frame()
{
	framesMap.erase(name);

	if (this == minibufferFrame)
	{
		minibufferFrame = nullptr;
	}
}

Frame::Frame(Frame&& other)
	: name(std::move(other.name)),
	  x(other.x), y(other.y), width(other.width), height(other.height),
	  currentBuffer(other.currentBuffer)
{
	framesMap[name] = this;
	other.name = "";

	if (currentBuffer->type == BufferType::MiniBuffer)
	{
		minibufferFrame = this;
	}
}

Frame& Frame::operator=(Frame&& other)
{
	if (this != &other)
	{
		framesMap.erase(name);

		name = other.name;
		x = other.x;
		y = other.y;
		width = other.width;
		height = other.height;
		currentBuffer = other.currentBuffer;

		if (currentBuffer->type == BufferType::MiniBuffer)
		{
			minibufferFrame = this;
		}

		other.currentBuffer = nullptr;
		other.name = "";
	}

	return *this;
}

Frame* Frame::get(const std::string& name)
{
	auto result = framesMap.find(name);

	if (result != framesMap.end())
	{
		return result->second;
	}
	else
	{
		return nullptr;
	}
}

void Frame::makeActive()
{
	if (this != currentFrame)
	{
		previousFrame = currentFrame;
		currentFrame = this;
	}
}
