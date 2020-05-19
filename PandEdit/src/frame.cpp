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

void Frame::destroyBuffer(Buffer* buffer)
{
	if (!buffer)
	{
		buffer = currentBuffer;
	}

	if (!buffer)
	{
		return;
	}

	delete buffer;
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
	  parent(other.parent), childOne(other.childOne), childTwo(other.childTwo),
	  pcDimensions(std::move(other.pcDimensions)),
	  windowWidth(other.windowWidth), windowHeight(other.windowHeight),
	  currentBuffer(other.currentBuffer), point(other.point), topLine(other.topLine)
{
	framesMap[name] = this;
	other.name = "";
	
	other.parent = nullptr;
	other.childOne = nullptr;
	other.childTwo = nullptr;

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

		parent = other.parent;
		childOne = other.childOne;
		childTwo = other.childTwo;
		other.parent = nullptr;
		other.childOne = nullptr;
		other.childTwo = nullptr;

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

void Frame::updateWindowSize(unsigned int newWidth, unsigned int newHeight, Font* font)
{
	windowWidth = newWidth;
	windowHeight = newHeight;
	getNumberOfLines(font);
}

void Frame::split(bool vertically)
{
	// TODO(fkp): Make sure it's not the minibuffer
	childOne = new Frame(name + "_C1", pcDimensions, windowWidth, windowHeight, currentBuffer, false);
	childTwo = new Frame(name + "_C2", pcDimensions, windowWidth, windowHeight, currentBuffer, true);

	// Sets the proper dimensions for the new frames
	if (vertically)
	{
		childOne->pcDimensions.width /= 2.0f;
		childTwo->pcDimensions.width /= 2.0f;
		childTwo->pcDimensions.x += childTwo->pcDimensions.width;
	}
	else
	{
		childOne->pcDimensions.height /= 2.0f;
		childTwo->pcDimensions.height /= 2.0f;
		childTwo->pcDimensions.y += childTwo->pcDimensions.height;
	}
	
	// Both children get copies of the frame data
	childOne->parent = childTwo->parent = this;
	childOne->point = childTwo->point = point;
	childOne->mark = childTwo->mark = mark;
	childOne->topLine = childTwo->topLine = topLine;

	// Invalidates everything for this frame
	currentBuffer = nullptr;
	point = Point {};
	mark = Point {};
	topLine = 0;

	// Adds the new frames to the window's map
	allFrames->push_back(childOne);
	allFrames->push_back(childTwo);
}

void Frame::destroy()
{
	if (!parent)
	{
		return;
	}

	Frame* sibling;

	if (this == parent->childOne)
	{
		sibling = parent->childTwo;
	}
	else
	{
		sibling = parent->childOne;
	}

	// TODO(fkp): This will work for bottom-level frames only. Maybe
	// check if the frame has chidren first?
	parent->deleteChildFrames(sibling);
}

void Frame::deleteChildFrames(Frame* otherChild)
{
	if (!childOne || !childTwo)
	{
		printf("Error: Frame is not a parent.\n");
		return;
	}

	allFrames->erase(std::remove(allFrames->begin(), allFrames->end(), childOne), allFrames->end());
	allFrames->erase(std::remove(allFrames->begin(), allFrames->end(), childTwo), allFrames->end());

	if (otherChild->childOne || otherChild->childTwo)
	{
		// NOTE(fkp): This weird deletion order is because otherChild
		// is one of this Frame's children.
		if (otherChild == childOne)
		{
			delete childTwo;
		}
		else if (otherChild == childTwo)
		{
			delete childOne;
		}
		else
		{
			printf("Error: otherChild is not one of this frame's children.\n");
			return;
		}

		childOne = otherChild->childOne;
		childOne->parent = this;
		childTwo = otherChild->childTwo;
		childTwo->parent = this;

		delete otherChild;
		otherChild = nullptr;

		resizeChildrenToFitSize();
		childOne->makeActive();
	}
	else
	{
		currentBuffer = otherChild->currentBuffer;
		point = otherChild->point;
		mark = otherChild->mark;
		topLine = otherChild->topLine;

		makeActive();

		delete childOne;
		delete childTwo;
		childOne = nullptr;
		childTwo = nullptr;
	}
}

void Frame::resizeChildrenToFitSize()
{

	if (childOne->pcDimensions.x != childTwo->pcDimensions.x)
	{
		// Was split vertically
		float childOneWidthPercent = childOne->pcDimensions.width / (childOne->pcDimensions.width + childTwo->pcDimensions.width);
		float childTwoWidthPercent = childTwo->pcDimensions.width / (childOne->pcDimensions.width + childTwo->pcDimensions.width);

		// Both frames still need to take up entire y-axis area
		childOne->pcDimensions.y = pcDimensions.y;
		childOne->pcDimensions.height = pcDimensions.height;
		childTwo->pcDimensions.y = pcDimensions.y;
		childTwo->pcDimensions.height = pcDimensions.height;

		childOne->pcDimensions.x = pcDimensions.x;
		childOne->pcDimensions.width = pcDimensions.width * childOneWidthPercent;
		childTwo->pcDimensions.x = pcDimensions.x + childOne->pcDimensions.width;
		childTwo->pcDimensions.width = pcDimensions.width * childTwoWidthPercent;
	}
	else if (childOne->pcDimensions.y != childTwo->pcDimensions.y)
	{
		// Was split horizontally
		float childOneHeightPercent = childOne->pcDimensions.height / (childOne->pcDimensions.height + childTwo->pcDimensions.height);
		float childTwoHeightPercent = childTwo->pcDimensions.height / (childOne->pcDimensions.height + childTwo->pcDimensions.height);
		
		// Both frames still need to take up entire x-axis area
		childOne->pcDimensions.x = pcDimensions.x;
		childOne->pcDimensions.width = pcDimensions.width;
		childTwo->pcDimensions.x = pcDimensions.x;
		childTwo->pcDimensions.width = pcDimensions.width;
		
		childOne->pcDimensions.y = pcDimensions.y;
		childOne->pcDimensions.height = pcDimensions.height * childOneHeightPercent;
		childTwo->pcDimensions.y = pcDimensions.y + childOne->pcDimensions.height;
		childTwo->pcDimensions.height = pcDimensions.height * childTwoHeightPercent;
	}

	if (childOne->childOne || childOne->childTwo)
	{
		// First child frame has children of its own
		childOne->resizeChildrenToFitSize();
	}

	if (childTwo->childOne || childTwo->childTwo)
	{
		// Second child frame has children of its own
		childTwo->resizeChildrenToFitSize();
	}
}

void Frame::getNumberOfLines(Font* currentFont)
{
	unsigned int pixelHeight = (unsigned int) (pcDimensions.height * windowHeight) - currentFont->size;
	numberOfLinesInView = pixelHeight / currentFont->size;
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
	std::string text = getTextPointToMark();
	
	if (appendToKillRing)
	{
		// TODO(fkp): Pass in text to avoid getting it twice
		copyRegion();
	}
	
	auto startAndEnd = getPointStartAndEnd();
	Point start = startAndEnd.first;
	Point end = startAndEnd.second;
	point = end;

	bool oldShouldAddInformation = currentBuffer->shouldAddToUndoInformation;
	currentBuffer->shouldAddToUndoInformation = false;
	
	while (point > start)
	{
		backspaceChar();
	}

	point = start;
	mark = start;
	
	currentBuffer->shouldAddToUndoInformation = oldShouldAddInformation;
	currentBuffer->addActionToUndoBuffer(Action::deletion(start, end, text));
}

void Frame::deleteRestOfLine()
{
	unsigned int numberOfCharsToDelete = currentBuffer->data[point.line].size() - point.col;
	
	if (numberOfCharsToDelete == 0)
	{
		// Will delete the newline if already at the end of the line
		numberOfCharsToDelete = 1;
	}
	
	deleteChar(numberOfCharsToDelete, true);
}

void Frame::doCommonPointManipulationTasks()
{
	pointFlashTimer.reset();

	if (currentBuffer->type != BufferType::MiniBuffer)
	{
		Frame::minibufferFrame->currentBuffer->data[0] = "";
		Frame::minibufferFrame->point.col = 0;
	}

	if (point.line < topLine || point.line > topLine + numberOfLinesInView)
	{
		centerPoint();
	}
}

void Frame::doCommonBufferManipulationTasks()
{
	// TODO(fkp): Figure out which lex mode to use
	if (currentBuffer->isUsingSyntaxHighlighting)
	{
		currentBuffer->lexer.lex(point.line, false);
	}
}

void Frame::insertChar(char character)
{
	Point startLocation = point;

	currentBuffer->data[point.line].insert(currentBuffer->data[point.line].begin() + point.col, character);
	point.col += 1;
	point.targetCol = point.col;

	currentBuffer->addActionToUndoBuffer(Action::insertion(startLocation, point, std::string(1, character)));

	adjustOtherFramePointLocations(true, false);
	doCommonPointManipulationTasks();
	doCommonBufferManipulationTasks();
}

void Frame::backspaceChar(unsigned int num, bool copyText)
{
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

				if (currentBuffer->isUsingSyntaxHighlighting)
				{
					currentBuffer->lexer.removeLine(point);
				}
				
				adjustOtherFramePointLocations(false, true);
			}
		}
	}

	if (copyText)
	{
		copyRegion(textDeleted);
	}

	currentBuffer->addActionToUndoBuffer(Action::deletion(point, endLocation, std::move(textDeleted)));	
	point.targetCol = point.col;
	
	doCommonPointManipulationTasks();
	doCommonBufferManipulationTasks();
}

void Frame::deleteChar(unsigned int num, bool copyText)
{
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

				if (currentBuffer->isUsingSyntaxHighlighting)
				{
					currentBuffer->lexer.removeLine(point);
				}

				adjustOtherFramePointLocations(false, true);
			}
		}
	}

	if (copyText)
	{
		copyRegion(textDeleted);
	}

	currentBuffer->addActionToUndoBuffer(Action::deletion(point, point, std::move(textDeleted)));
	point.targetCol = point.col;
	
	doCommonPointManipulationTasks();
	doCommonBufferManipulationTasks();
}

void Frame::newLine()
{
	Point startLocation = point;

	std::string restOfLine { currentBuffer->data[point.line].begin() + point.col, currentBuffer->data[point.line].end() };
	currentBuffer->data[point.line].erase(currentBuffer->data[point.line].begin() + point.col, currentBuffer->data[point.line].end());

	point.line += 1;
	point.col = 0;
	point.targetCol = point.col;

	currentBuffer->data.insert(currentBuffer->data.begin() + point.line, restOfLine);
	currentBuffer->addActionToUndoBuffer(Action::insertion(startLocation, point, std::string(1, '\n')));

	if (currentBuffer->isUsingSyntaxHighlighting)
	{
		currentBuffer->lexer.addLine(startLocation);
	}

	// TODO(fkp): This is a hack
	point.line -= 1;
	doCommonBufferManipulationTasks();
	point.line += 1;
	
	doCommonPointManipulationTasks();
	adjustOtherFramePointLocations(true, true);
}

void Frame::insertString(const std::string& string)
{
	Point startLocation = point;
	bool oldShouldAddInformation = currentBuffer->shouldAddToUndoInformation;
	unsigned int oldTopLine = topLine;
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

	if (topLine != oldTopLine)
	{
		centerPoint();
	}

	currentBuffer->shouldAddToUndoInformation = oldShouldAddInformation;
	currentBuffer->addActionToUndoBuffer(Action::insertion(startLocation, point, string));
}

void Frame::movePointLeft(unsigned int num)
{
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
	doCommonPointManipulationTasks();
}

void Frame::movePointRight(unsigned int num)
{
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
	doCommonPointManipulationTasks();
}

void Frame::movePointUp()
{
	if (point.line > 0)
	{
		point.line -= 1;
		moveColToTarget();
	}
	
	doCommonPointManipulationTasks();
}

void Frame::movePointDown()
{
	if (point.line < currentBuffer->data.size() - 1)
	{
		point.line += 1;
		moveColToTarget();
	}

	doCommonPointManipulationTasks();
}

void Frame::movePointHome()
{
	if (currentBuffer->type == BufferType::MiniBuffer)
	{
		// Should move to after the 'Execute: ' part
		point.col = Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' ') + 1;
	}
	else
	{
		point.col = 0;
	}

	doCommonPointManipulationTasks();
	point.targetCol = point.col;
}

void Frame::movePointEnd()
{
	point.col = currentBuffer->data[point.line].size();
	point.targetCol = point.col;
	
	doCommonPointManipulationTasks();
}

void Frame::movePointToBufferStart()
{
	point.line = 0;
	point.col = 0;
	point.targetCol = 0;

	doCommonPointManipulationTasks();
}

void Frame::movePointToBufferEnd()
{
	point.line = currentBuffer->data.size() - 1;
	point.col = currentBuffer->data[point.line].size();
	point.targetCol = point.col;
	
	doCommonPointManipulationTasks();
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

void Frame::centerPoint()
{
	int numberOfLinesToMove = point.line - ((int) topLine + (numberOfLinesInView / 2));
	moveView(numberOfLinesToMove, false);
}

#define WORD_SEPARATORS "`~!@#$%^&*()-=+[]{}\\|;:'\",.<>/?"

unsigned int Frame::findWordBoundaryLeft()
{
	static std::string wordSeparators = WORD_SEPARATORS;
	unsigned int numberOfChars = 0;

	while (point.col - numberOfChars > 0)
	{
		// Stops at word boundary, only if not the first character
		if (numberOfChars != 0)
		{
			if (currentBuffer->data[point.line][point.col - numberOfChars - 1] == ' ')
			{
				break;
			}
			else if (wordSeparators.find(currentBuffer->data[point.line][point.col - numberOfChars - 1]) != std::string::npos)
			{
				break;
			}
		}

		numberOfChars += 1;
	}

	return numberOfChars;
}

unsigned int Frame::findWordBoundaryRight()
{
	static std::string wordSeparators = WORD_SEPARATORS;
	unsigned int numberOfChars = 0;

	while (point.col + numberOfChars < currentBuffer->data[point.line].size())
	{
		// Stops at word boundary, only if not the first character
		if (numberOfChars != 0)
		{
			if (currentBuffer->data[point.line][point.col + numberOfChars] == ' ')
			{
				break;
			}
			else if (wordSeparators.find(currentBuffer->data[point.line][point.col + numberOfChars]) != std::string::npos)
			{
				break;
			}
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
	for (Frame* frame : *allFrames)
	{
		if (frame == this) continue;

		if (currentBuffer == frame->currentBuffer)
		{
			if (insertion)
			{
				if (point.line == frame->point.line)
				{
					if (lineWrap)
					{
						frame->point.line += 1;
					}
					else
					{
						if (point.col <= frame->point.col + 1)
						{
							frame->point.col += 1;
						}
					}
				}
				else if (point.line == frame->point.line + 1)
				{
					if (lineWrap)
					{
						if (frame->point.col >= frame->currentBuffer->data[frame->point.line].size())
						{
							frame->point.col -= frame->currentBuffer->data[frame->point.line].size();
							frame->point.line += 1;
						}
					}
				}
				else if (point.line < frame->point.line)
				{
					if (lineWrap)
					{
						frame->point.line += 1;
					}
				}
			}
			else
			{
				if (point.line == frame->point.line)
				{
					if (lineWrap)
					{
						frame->point.col += point.col;
					}
					else
					{
						if (point.col < frame->point.col)
						{
							frame->point.col -= 1;
						}
					}
				}
				else if (point.line == frame->point.line - 1)
				{
					if (lineWrap)
					{
						frame->point.line -= 1;
						frame->point.col += point.col;
					}
				}
				else if (point.line < frame->point.line - 1)
				{
					if (lineWrap)
					{
						frame->point.line -= 1;
					}
				}
			}
		}
	}
}

void Frame::copyRegion(std::string text)
{
	std::string textToCopy;
	
	// TODO(fkp): This might break if text is supposed to be empty
	if (text == "")
	{
		textToCopy = getTextPointToMark();
	}
	else
	{
		// TODO(fkp): Avoid the copy
		textToCopy = text;
	}
	
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
