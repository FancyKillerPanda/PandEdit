//  ===== Date Created: 15 April, 2020 =====

#define NOMINMAX
#include <windows.h>
#include <algorithm>

#include "frame.hpp"
#include "font.hpp"
#include "undo.hpp"

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
		currentBuffer->lastTopLine = topLine;
	}

	currentBuffer = buffer;
	point = currentBuffer->lastPoint;
	topLine = currentBuffer->lastTopLine;
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
	  currentBuffer(other.currentBuffer), point(other.point), topLine(other.topLine)
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
		topLine = other.topLine;
		
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
	unsigned int pixelHeight = (unsigned int) (pcDimensions.height * windowHeight) - currentFont->size;
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
		copyRegion();
	}
	
	auto startAndEnd = getPointStartAndEnd();
	Point start = startAndEnd.first;
	Point end = startAndEnd.second;
	point = end;
	
	while (point > start)
	{
		backspaceChar();
	}

	point = start;
	mark = start;
}

void Frame::deleteRestOfLine()
{
	unsigned int numberOfCharsToDelete = currentBuffer->data[point.line].size() - point.col;
	
	if (numberOfCharsToDelete == 0)
	{
		// Will delete the newline if already at the end of the line
		numberOfCharsToDelete = 1;
	}
	
	deleteChar(numberOfCharsToDelete);
}

void Frame::doCommonPointManipulationTasks()
{
	pointFlashFrameCounter = 0;

	if (currentBuffer->type != BufferType::MiniBuffer)
	{
		Frame::minibufferFrame->currentBuffer->data[0] = "";
		Frame::minibufferFrame->point.col = 0;
	}
}

void Frame::insertChar(char character)
{
	doCommonPointManipulationTasks();
	Point startLocation = point;

	currentBuffer->data[point.line].insert(currentBuffer->data[point.line].begin() + point.col, character);
	point.col += 1;
	point.targetCol = point.col;

	currentBuffer->addActionToUndoBuffer(Action::insertion(startLocation, point, std::string(1, character)));

	adjustOtherFramePointLocations(true, false);
}

void Frame::backspaceChar(unsigned int num)
{
	doCommonPointManipulationTasks();
	std::string textDeleted = "";
	Point endLocation = point; // This is not startLocation because we are going backwards

	if (num == 0) num = 1;

	for (int i = 0; i < num; i++)
	{
		if (point.col > 0)
		{
			if (currentBuffer->type == BufferType::MiniBuffer &&
				point.col <= Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' ') + 1)
			{
				// Should not be able to backspace into the 'Execute: ' part
				break;
			}

			point.col -= 1;
			textDeleted.insert(0, currentBuffer->data[point.line].substr(point.col, 1));
			currentBuffer->data[point.line].erase(point.col, 1);

			adjustOtherFramePointLocations(false, false);
		}
		else
		{
			if (point.line > 0)
			{
				point.line -= 1;
				point.col = currentBuffer->data[point.line].size();

				textDeleted.insert(0, "\n");
				currentBuffer->data[point.line] += currentBuffer->data[point.line + 1];
				currentBuffer->data.erase(currentBuffer->data.begin() + point.line + 1);

				adjustOtherFramePointLocations(false, true);
			}
		}
	}

	currentBuffer->addActionToUndoBuffer(Action::deletion(point, endLocation, std::move(textDeleted)));	
	point.targetCol = point.col;
}

void Frame::deleteChar(unsigned int num)
{
	doCommonPointManipulationTasks();
	std::string textDeleted = "";

	if (num == 0) num = 1;

	for (int i = 0; i < num; i++)
	{
		if (point.col < currentBuffer->data[point.line].size())
		{
			textDeleted += currentBuffer->data[point.line].substr(point.col, 1);
			currentBuffer->data[point.line].erase(point.col, 1);
			adjustOtherFramePointLocations(false, false);
		}
		else
		{
			if (point.line < currentBuffer->data.size() - 1)
			{
				textDeleted += '\n';
				
				currentBuffer->data[point.line] += currentBuffer->data[point.line + 1];
				currentBuffer->data.erase(currentBuffer->data.begin() + point.line + 1);
				adjustOtherFramePointLocations(false, true);
			}
		}
	}

	currentBuffer->addActionToUndoBuffer(Action::deletion(point, point, std::move(textDeleted)));
	point.targetCol = point.col;
}

void Frame::newLine()
{
	doCommonPointManipulationTasks();
	Point startLocation = point;

	std::string restOfLine { currentBuffer->data[point.line].begin() + point.col, currentBuffer->data[point.line].end() };
	currentBuffer->data[point.line].erase(currentBuffer->data[point.line].begin() + point.col, currentBuffer->data[point.line].end());

	point.line += 1;
	point.col = 0;
	point.targetCol = point.col;

	currentBuffer->data.insert(currentBuffer->data.begin() + point.line, restOfLine);
	currentBuffer->addActionToUndoBuffer(Action::insertion(startLocation, point, std::string(1, '\n')));
	adjustOtherFramePointLocations(true, true);
}

void Frame::insertString(const std::string& string)
{
	Point startLocation = point;
	bool oldShouldAddInformation = currentBuffer->shouldAddToUndoInformation;
	currentBuffer->shouldAddToUndoInformation = false;

	for (char character : string)
	{
		if (character == '\n')
		{
			newLine();
		}
		else if (character != '\r') // Windows has CRLF endings
		{
			insertChar(character);
		}
	}

	currentBuffer->shouldAddToUndoInformation = oldShouldAddInformation;
	currentBuffer->addActionToUndoBuffer(Action::insertion(startLocation, point, string));
}

void Frame::movePointLeft(unsigned int num)
{
	doCommonPointManipulationTasks();

	if (num == 0) num = 1;

	for (int i = 0; i < num; i++)
	{
		if (point.col > 0)
		{
			if (currentBuffer->type == BufferType::MiniBuffer &&
				point.col <= Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' ') + 1)
			{
				// Should not be able to move into the 'Execute: ' part
				break;
			}

			point.col -= 1;
		}
		else
		{
			if (point.line > 0)
			{
				point.line -= 1;
				point.col = currentBuffer->data[point.line].size();
			}
		}
	}

	point.targetCol = point.col;
}

void Frame::movePointRight(unsigned int num)
{
	doCommonPointManipulationTasks();

	if (num == 0) num = 1;

	for (int i = 0; i < num; i++)
	{
		if (point.col < currentBuffer->data[point.line].size())
		{
			point.col += 1;
		}
		else
		{
			if (point.line < currentBuffer->data.size() - 1)
			{
				point.line += 1;
				point.col = 0;
			}
		}
	}

	point.targetCol = point.col;
}

void Frame::movePointUp(Font* currentFont)
{
	doCommonPointManipulationTasks();

	if (point.line > 0)
	{
		point.line -= 1;
		moveColToTarget();
	}

	if (point.line < topLine)
	{
		centerPoint(currentFont);
	}
}

void Frame::movePointDown(Font* currentFont)
{
	doCommonPointManipulationTasks();

	if (point.line < currentBuffer->data.size() - 1)
	{
		point.line += 1;
		moveColToTarget();
	}

	if (point.line >= topLine + getNumberOfLines(currentFont))
	{
		centerPoint(currentFont);
	}
}

void Frame::movePointHome()
{
	doCommonPointManipulationTasks();

	if (currentBuffer->type == BufferType::MiniBuffer)
	{
		// Should move to after the 'Execute: ' part
		point.col = Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' ') + 1;
	}
	else
	{
		point.col = 0;
	}

	point.targetCol = point.col;
}

void Frame::movePointEnd()
{
	doCommonPointManipulationTasks();

	point.col = currentBuffer->data[point.line].size();
	point.targetCol = point.col;
}

void Frame::moveView(int numberOfLines, bool movePoint)
{
	unsigned int oldLineTop = topLine;
	int newLineTop = (int) topLine + numberOfLines;

	if (newLineTop < 0)
	{
		newLineTop = 0;
	}	
	
	if (newLineTop > currentBuffer->data.size() - 2)
	{
		newLineTop = currentBuffer->data.size() - 2;
	}

	topLine = newLineTop;;
	int numberOfLinesMoved = (int) topLine - (int) oldLineTop;

	if (movePoint)
	{
		point.line += numberOfLinesMoved;

		if (point.line < 0)
		{
			point.line = 0;
		}

		if (point.line > currentBuffer->data.size() - 2)
		{
			point.line = currentBuffer->data.size() - 2;
		}
	}
}

void Frame::centerPoint(Font* currentFont)
{
	unsigned int numberOfFrameLines = getNumberOfLines(currentFont);
	int numberOfLinesToMove = point.line - ((int) topLine + (numberOfFrameLines / 2));
	moveView(numberOfLinesToMove, false);
}

unsigned int Frame::findWordBoundaryLeft()
{
	unsigned int numberOfChars = 0;

	while (point.col - numberOfChars > 0)
	{
		// Stops at space, only if not the first character
		if (numberOfChars != 0 &&
			currentBuffer->data[point.line][point.col - numberOfChars - 1] == ' ')
		{
			break;
		}

		numberOfChars += 1;
	}

	return numberOfChars;
}

unsigned int Frame::findWordBoundaryRight()
{
	unsigned int numberOfChars = 0;

	while (point.col + numberOfChars < currentBuffer->data[point.line].size())
	{
		// Stops at space, only if not the first character
		if (numberOfChars != 0 &&
			currentBuffer->data[point.line][point.col + numberOfChars] == ' ')
		{
			break;
		}

		numberOfChars += 1;
	}

	return numberOfChars;
}

void Frame::moveColToTarget()
{
	if (point.col > currentBuffer->data[point.line].size())
	{
		point.col = currentBuffer->data[point.line].size();
	}
	else
	{
		if (point.targetCol > currentBuffer->data[point.line].size())
		{
			point.col = currentBuffer->data[point.line].size();
		}
		else
		{
			point.col = point.targetCol;
		}
	}
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

void Frame::copyRegion()
{
	std::string textToCopy = getTextPointToMark();
	
	if (!OpenClipboard(GetDesktopWindow()))
	{
		printf("Error: Failed to open clipboard for copying to.\n");
		return;
	}

	EmptyClipboard();
	HGLOBAL clipboardData = GlobalAlloc(GMEM_MOVEABLE, textToCopy.size() + 1);

	if (!clipboardData)
	{
		printf("Error: Failed to allocate global memory for text.\n");
		CloseClipboard();

		return;
	}

	memcpy(GlobalLock(clipboardData), textToCopy.c_str(), textToCopy.size() + 1);
	GlobalUnlock(clipboardData);
	SetClipboardData(CF_TEXT, clipboardData);
	CloseClipboard();
	GlobalFree(clipboardData);

	// TODO(fkp): Kill ring size limiting
	if (killRing.size() == 0 || textToCopy != killRing.back())
	{
		killRing.push_back(std::move(textToCopy));
		killRingPointer = killRing.size() - 1;
	}
	
	lastClipboardSequenceNumber = GetClipboardSequenceNumber();
}

void Frame::paste()
{
	if (GetClipboardSequenceNumber() != lastClipboardSequenceNumber)
	{
		pasteClipboard();
	}
	else
	{
		// TODO(fkp): Use the kill ring pointer
		if (killRing.size() > 0)
		{
			mark.line = point.line;
			mark.col = point.col;
			
			insertString(killRing[killRingPointer]);
		}
	}
}

void Frame::pasteClipboard()
{
	if (!IsClipboardFormatAvailable(CF_TEXT))
	{
		printf("Error: Pasting text not supported.\n");
		return;
	}

	if (!OpenClipboard(GetDesktopWindow()))
	{
		printf("Error: Failed to open clipboard for pasting.\n");
		return;
	}

	HGLOBAL clipboardData = GetClipboardData(CF_TEXT);

	if (clipboardData)
	{
		LPCSTR clipboardString = (LPCSTR) GlobalLock(clipboardData);;

		if (clipboardString)
		{
			mark.line = point.line;
			mark.col = point.col;
			
			insertString(clipboardString);
			GlobalUnlock(clipboardData);

			killRing.push_back(std::string { clipboardString });
			killRingPointer = killRing.size() - 1;
		}
	}

	CloseClipboard();
}

void Frame::pastePop()
{
	deleteTextPointToMark(false);
	killRingPointer -= 1;
	
	if (killRingPointer < 0)
	{
		killRingPointer = killRing.size() - 1;
		
		if (killRing.size() == 0)
		{
			return;
		}
	}
	
	paste();
}
