//  ===== Date Created: 15 April, 2020 =====

#include "frame.hpp"

Frame* Frame::currentFrame = nullptr;
Frame* Frame::previousFrame = nullptr;
Frame* Frame::minibufferFrame = nullptr;
std::unordered_map<std::string, Frame*> Frame::framesMap;

Frame::Frame(std::string name, Vector4f dimensions, unsigned int windowWidth, unsigned int windowHeight, Buffer* buffer, bool isActive)
{
	init(name, dimensions, buffer, isActive);
}

Frame::Frame(std::string name, Vector4f dimensions, unsigned int windowWidth, unsigned int windowHeight, BufferType type, std::string bufferName, bool isActive)
	: windowWidth(windowWidth), windowHeight(windowHeight)
{
	Buffer* buffer = new Buffer { type, bufferName };
	init(name, dimensions, buffer, isActive);
}

void Frame::init(std::string name, Vector4f dimensions, Buffer* buffer, bool isActive)
{
	this->name = name;
	currentBuffer = buffer;
	
	// NOTE(fkp): windowWidth and windowHeight initialised in constructors
	pcDimensions = dimensions;
	
	line = currentBuffer->lastLine;
	col = currentBuffer->lastCol;
	targetCol = currentBuffer->lastTargetCol;
	
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
	  pcDimensions(std::move(other.pcDimensions)),
	  windowWidth(other.windowWidth), windowHeight(other.windowHeight),
	  currentBuffer(other.currentBuffer),
	  line(other.line), col(other.col), targetCol(other.targetCol)
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
		
		pcDimensions = other.pcDimensions;
		windowWidth = other.windowWidth;
		windowHeight = other.windowHeight;
		
		currentBuffer = other.currentBuffer;
		line = other.line;
		col = other.col;
		targetCol = other.targetCol;

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

void Frame::updateWindowSize(unsigned int newWidth, unsigned int newHeight)
{
	windowWidth = newWidth;
	windowHeight = newHeight;
}

Frame* Frame::splitVertically()
{
	pcDimensions.width /= 2.0f;

	Vector4f newFrameDimensions = pcDimensions;
	newFrameDimensions.x += pcDimensions.width;

	std::string newFrameName = name + "_SplitRight";
	Frame* result = new Frame(newFrameName, pcDimensions, windowWidth, windowHeight, currentBuffer, true);

	result->line = line;
	result->col = col;
	result->targetCol = targetCol;

	return result;
}

Frame* Frame::splitHorizontally()
{
	pcDimensions.height /= 2.0f;

	Vector4f newFrameDimensions = pcDimensions;
	newFrameDimensions.y += pcDimensions.height;

	std::string newFrameName = name + "_SplitDown";
	Frame* result = new Frame(newFrameName, pcDimensions, windowWidth, windowHeight, currentBuffer, true);
	
	result->line = line;
	result->col = col;
	result->targetCol = targetCol;

	return result;
}
