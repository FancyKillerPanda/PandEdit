//  ===== Date Created: 15 April, 2020 =====

#include <algorithm>

#include "frame.hpp"

Frame* Frame::currentFrame = nullptr;
Frame* Frame::previousFrame = nullptr;
Frame* Frame::minibufferFrame = nullptr;
std::vector<Frame>* Frame::allFrames = nullptr;
std::unordered_map<std::string, Frame*> Frame::framesMap;

Frame::Frame(std::string name, Vector4f dimensions, unsigned int windowWidth, unsigned int windowHeight, Buffer* buffer, bool isActive)
{
	init(name, dimensions, windowWidth, windowHeight, buffer, isActive);
}

Frame::Frame(std::string name, Vector4f dimensions, unsigned int windowWidth, unsigned int windowHeight, BufferType type, std::string bufferName, bool isActive)
{
	Buffer* buffer = new Buffer { type, bufferName };
	init(name, dimensions, windowWidth, windowHeight, buffer, isActive);
}

void Frame::init(std::string name, Vector4f dimensions, unsigned int windowWidth, unsigned int windowHeight, Buffer* buffer, bool isActive)
{
	this->name = name;

	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	pcDimensions = dimensions;

	switchToBuffer(buffer);
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

void Frame::switchToBuffer(Buffer* buffer)
{
	if (currentBuffer)
	{
		currentBuffer->lastLine = line;
		currentBuffer->lastCol = col;
		currentBuffer->lastTargetCol = col; // Don't want to save the target col
	}

	currentBuffer = buffer;
	line = currentBuffer->lastLine;
	col = currentBuffer->lastCol;
	targetCol = currentBuffer->lastTargetCol;
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
	Frame* result = new Frame(newFrameName, newFrameDimensions, windowWidth, windowHeight, currentBuffer, true);

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
	Frame* result = new Frame(newFrameName, newFrameDimensions, windowWidth, windowHeight, currentBuffer, true);

	result->line = line;
	result->col = col;
	result->targetCol = targetCol;

	return result;
}

std::string Frame::getTextPointToMark()
{
	std::string result;
	int startLine;
	int endLine;
	int startCol;
	int endCol;

	if (line == markLine)
	{
		startLine = line;
		endLine = line;
		startCol = std::min(col, markCol);
		endCol = std::max(col, markCol);
	}
	else if (line < markLine)
	{
		startLine = line;
		endLine = markLine;
		startCol = col;
		endCol = markCol;
	}
	else
	{
		startLine = markLine;
		endLine = line;
		startCol = markCol;
		endCol = col;
	}

	for (int currentLine = startLine; currentLine <= endLine; currentLine++)
	{
		if (currentLine == startLine)
		{
			// Checks if the mark is on the same line as the point
			if (startLine == endLine)
			{
				result = currentBuffer->data[currentLine].substr(startCol, endCol - startCol);
			}
			else
			{
				result += currentBuffer->data[currentLine].substr(startCol, std::string::npos);
				result += '\n';
			}
		}
		else if (currentLine == endLine)
		{
			// NOTE(fkp): We don't need to check if line == markLine
			// here because this is in an else if from the previous
			// if.
			result += currentBuffer->data[currentLine].substr(0, endCol);
		}
		else
		{
			result += currentBuffer->data[currentLine];
			result += '\n';
		}
	}

	return std::move(result);
}

// TODO(fkp): Cleanup. This method is one big mess and probably
// riddled with bugs.
void Frame::adjustOtherFramePointLocations(bool insertion, bool lineWrap)
{
	for (Frame& frame : *allFrames)
	{
		if (&frame == this) continue;

		if (currentBuffer == frame.currentBuffer)
		{
			if (insertion)
			{
				if (line == frame.line)
				{
					if (lineWrap)
					{
						frame.line += 1;
					}
					else
					{
						if (col <= frame.col + 1)
						{
							frame.col += 1;
						}
					}
				}
				else if (line == frame.line + 1)
				{
					if (lineWrap)
					{
						if (frame.col >= frame.currentBuffer->data[frame.line].size())
						{
							frame.col -= frame.currentBuffer->data[frame.line].size();
							frame.line += 1;
						}
					}
				}
				else if (line < frame.line)
				{
					if (lineWrap)
					{
						frame.line += 1;
					}
				}
			}
			else
			{
				if (line == frame.line)
				{
					if (lineWrap)
					{
						frame.col += col;
					}
					else
					{
						if (col < frame.col)
						{
							frame.col -= 1;
						}
					}
				}
				else if (line == frame.line - 1)
				{
					if (lineWrap)
					{
						frame.line -= 1;
						frame.col += col;
					}
				}
				else if (line < frame.line - 1)
				{
					if (lineWrap)
					{
						frame.line -= 1;
					}
				}
			}
		}
	}
}
