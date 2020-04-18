//  ===== Date Created: 15 April, 2020 =====

#include "frame.hpp"

Frame* Frame::currentFrame = nullptr;
Frame* Frame::previousFrame = nullptr;
Frame* Frame::minibufferFrame = nullptr;
std::unordered_map<std::string, Frame*> Frame::framesMap;

Frame::Frame(std::string name, int x, int y, unsigned int width, unsigned int height, Buffer* buffer, bool isActive)
{
	init(name, x, y, width, height, buffer, isActive);
}

Frame::Frame(std::string name, int x, int y, unsigned int width, unsigned int height, BufferType type, std::string bufferName, bool isActive)
{
	Buffer* buffer = new Buffer { type, bufferName };
	init(name, x, y, width, height, buffer, isActive);
}

void Frame::init(std::string name, int x, int y, unsigned int width, unsigned int height, Buffer* buffer, bool isActive)
{
	this->name = name;
	this->currentBuffer = buffer;
	
	this->realX = x;
	this->realWidth = width;
	this->x = realX + FRAME_BORDER_WIDTH;
	this->y = y;
	this->width = realWidth - FRAME_BORDER_WIDTH;
	this->height = height;
	
	framesMap.insert({ name, this });

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
	  realX(other.realX), realWidth(other.realWidth),
	  x(other.x), y(other.y), width(other.width), height(other.height),
	  currentBuffer(other.currentBuffer)
{
	framesMap[name] = this;
	other.name = "";

	if (&other == Frame::currentFrame)
	{
		Frame::currentFrame = this;
	}

	// TODO(fkp): Should this be an else if?
	if (&other == Frame::previousFrame)
	{
		Frame::previousFrame = this;
	}
	
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
		realX = other.realX;
		realWidth = other.realWidth;
		x = other.x;
		y = other.y;
		width = other.width;
		height = other.height;
		currentBuffer = other.currentBuffer;

		if (&other == Frame::currentFrame)
		{
			Frame::currentFrame = this;
		}

		// TODO(fkp): Should this be an else if?
		if (&other == Frame::previousFrame)
		{
			Frame::previousFrame = this;
		}
	
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
