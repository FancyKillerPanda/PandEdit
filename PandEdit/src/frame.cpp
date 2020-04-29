//  ===== Date Created: 15 April, 2020 =====

#include <algorithm>

#include "frame.hpp"
#include "text.hpp"

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
	Buffer* buffer = new Buffer { type, bufferName, "" };
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
		currentBuffer->lastPoint = point;
		currentBuffer->lastPoint.targetCol = point.col; // Don't want to save the target col
	}

	currentBuffer = buffer;
	point = currentBuffer->lastPoint;
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
	  currentBuffer(other.currentBuffer), point(other.point)
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
		point = other.point;
		
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
	result->point = point;

	return result;
}

Frame* Frame::splitHorizontally()
{
	pcDimensions.height /= 2.0f;

	Vector4f newFrameDimensions = pcDimensions;
	newFrameDimensions.y += pcDimensions.height;

	std::string newFrameName = name + "_SplitDown";
	Frame* result = new Frame(newFrameName, newFrameDimensions, windowWidth, windowHeight, currentBuffer, true);
	result->point = point;

	return result;
}

unsigned int Frame::getNumberOfLines(Font* currentFont)
{
	unsigned int pixelHeight = (unsigned int) (pcDimensions.height * windowHeight);
	return pixelHeight / currentFont->size;
}

std::pair<Point, Point> Frame::getPointStartAndEnd()
{
	Point start { currentBuffer };
	Point end { currentBuffer };
	
	if (point.line == mark.line)
	{
		start.line = point.line;
		end.line = point.line;
		start.col = std::min(point.col, mark.col);
		end.col = std::max(point.col, mark.col);
	}
	else if (point.line < mark.line)
	{
		start.line = point.line;
		end.line = mark.line;
		start.col = point.col;
		end.col = mark.col;
	}
	else
	{
		start.line = mark.line;
		end.line = point.line;
		start.col = mark.col;
		end.col = point.col;
	}

	return { start, end };
}

std::string Frame::getTextPointToMark()
{
	std::string result;

	auto startAndEnd = getPointStartAndEnd();
	Point start = startAndEnd.first;
	Point end = startAndEnd.second;
	
	for (int currentLine = start.line; currentLine <= end.line; currentLine++)
	{
		if (currentLine == start.line)
		{
			// Checks if the mark is on the same line as the point
			if (start.line == end.line)
			{
				result = currentBuffer->data[currentLine].substr(start.col, end.col - start.col);
			}
			else
			{
				result += currentBuffer->data[currentLine].substr(start.col, std::string::npos);
				result += '\n';
			}
		}
		else if (currentLine == end.line)
		{
			// NOTE(fkp): We don't need to check if point.line == mark.line
			// here because this is in an else if from the previous
			// if.
			result += currentBuffer->data[currentLine].substr(0, end.col);
		}
		else
		{
			result += currentBuffer->data[currentLine];
			result += '\n';
		}
	}

	return std::move(result);
}

void Frame::deleteTextPointToMark(bool appendToKillRing)
{
	if (appendToKillRing)
	{
		currentBuffer->copyRegion(*this);
	}
	
	auto startAndEnd = getPointStartAndEnd();
	Point start = startAndEnd.first;
	Point end = startAndEnd.second;
	point = end;
	
	while (point > start)
	{
		currentBuffer->backspaceChar(*this);
	}

	point = start;
	mark = start;
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
				if (point.line == frame.point.line)
				{
					if (lineWrap)
					{
						frame.point.line += 1;
					}
					else
					{
						if (point.col <= frame.point.col + 1)
						{
							frame.point.col += 1;
						}
					}
				}
				else if (point.line == frame.point.line + 1)
				{
					if (lineWrap)
					{
						if (frame.point.col >= frame.currentBuffer->data[frame.point.line].size())
						{
							frame.point.col -= frame.currentBuffer->data[frame.point.line].size();
							frame.point.line += 1;
						}
					}
				}
				else if (point.line < frame.point.line)
				{
					if (lineWrap)
					{
						frame.point.line += 1;
					}
				}
			}
			else
			{
				if (point.line == frame.point.line)
				{
					if (lineWrap)
					{
						frame.point.col += point.col;
					}
					else
					{
						if (point.col < frame.point.col)
						{
							frame.point.col -= 1;
						}
					}
				}
				else if (point.line == frame.point.line - 1)
				{
					if (lineWrap)
					{
						frame.point.line -= 1;
						frame.point.col += point.col;
					}
				}
				else if (point.line < frame.point.line - 1)
				{
					if (lineWrap)
					{
						frame.point.line -= 1;
					}
				}
			}
		}
	}
}
