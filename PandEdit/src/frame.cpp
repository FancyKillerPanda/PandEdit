//  ===== Date Created: 15 April, 2020 =====

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <algorithm>
#include <filesystem>

#include "frame.hpp"
#include "font.hpp"
#include "undo.hpp"
#include "commands.hpp"

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

	if (buffer)
	{
		switchToBuffer(buffer);
	}
	
	framesMap.insert({ name, this });

	if (isActive)
	{
		makeActive();
	}

	if (currentBuffer && currentBuffer->type == BufferType::MiniBuffer)
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
		currentBuffer->lastTopLine = targetTopLine;
	}

	currentBuffer = buffer;
	point = currentBuffer->lastPoint;
	targetTopLine = currentBuffer->lastTopLine;
	currentTopLine = targetTopLine;
	popupLines.clear();
	popupCurrentSuggestion = 0;
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
	: parent(other.parent), childOne(other.childOne), childTwo(other.childTwo),
	  name(std::move(other.name)),
	  pcDimensions(std::move(other.pcDimensions)),
	  windowWidth(other.windowWidth), windowHeight(other.windowHeight),
	  currentBuffer(other.currentBuffer), point(other.point),
	  currentTopLine(other.currentTopLine), targetTopLine(other.targetTopLine)
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
		targetTopLine = other.targetTopLine;
		currentTopLine = other.currentTopLine;
		
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

	if (point.line > targetTopLine + numberOfLinesInView)
	{
		centerPoint();
	}
}

void Frame::split(bool vertically, Font* currentFont)
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
	childOne->targetTopLine = childTwo->targetTopLine = targetTopLine;
	childOne->currentTopLine = childTwo->currentTopLine = currentTopLine;

	// Calculates how many lines are in view
	childOne->getNumberOfLines(currentFont);
	childTwo->getNumberOfLines(currentFont);

	if (childOne->point.line > childOne->targetTopLine + childOne->numberOfLinesInView)
	{
		childOne->centerPoint();
	}
	
	if (childTwo->point.line > childTwo->targetTopLine + childTwo->numberOfLinesInView)
	{
		childTwo->centerPoint();
	}
	
	// Invalidates everything for this frame
	currentBuffer = nullptr;
	point = Point {};
	mark = Point {};
	targetTopLine = 0;
	currentTopLine = 0;

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
		targetTopLine = otherChild->targetTopLine;
		currentTopLine = otherChild->currentTopLine;

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

	return result;
}

void Frame::deleteTextPointToMark(bool appendToKillRing)
{
	if (!warnIfBufferIsReadOnly()) return;
	
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
	if (!warnIfBufferIsReadOnly()) return;
	
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

	if (point.line < targetTopLine || point.line + 1 > targetTopLine + numberOfLinesInView)
	{
		centerPoint();
	}

	// This is to display the signature of the function that is being hovered over
	Token* tokenUnderPoint = getTokenUnderPoint();
	
	if (tokenUnderPoint)
	{
		LineLexState& lineState = currentBuffer->lexer.lineStates[tokenUnderPoint->start.line];
		int indexOfTokenUnderPoint = lineState.getIndexOfToken(tokenUnderPoint);
		int currentIndex = indexOfTokenUnderPoint + 1;
		int parenCount = 0; // Reduces on close paren, increases on open paren

		while (Token* tokenBefore = lineState.getTokenBefore(currentIndex, EXCLUDE_NONE))
		{
			if (tokenBefore->type == Token::Type::FunctionUsage)
			{
				// The second bit is there because if the point is
				// directly over the function name, parenCount will be
				// equal to 0 (because the parenthesis is on the right).
				if (parenCount > 0 || currentIndex == indexOfTokenUnderPoint + 1)
				{	
					auto function = currentBuffer->functionDefinitions.find(currentBuffer->substrFromPoints(tokenBefore->start, tokenBefore->end));
		
					// TODO(fkp): Standard library functions
					if (function != currentBuffer->functionDefinitions.end())
					{
						printf("Function: '%s'\n", function->second.c_str());
					}
				}

				break;
			}
			else if (tokenBefore->type == Token::Type::LeftParen)
			{
				parenCount += 1;
			}
			else if (tokenBefore->type == Token::Type::RightParen)
			{
				parenCount -= 1;
			}

			currentIndex -= 1;
		}
	}
	
	popupLines.clear();
	// TODO(fkp): This doesn't work because a '\t' is inserted
	// before the suggestion is completed.
	// popupCurrentSuggestion = 0;
}

void Frame::doCommonBufferManipulationTasks()
{
	if (!warnIfBufferIsReadOnly()) return;
	
	// TODO(fkp): Figure out which lex mode to use
	if (currentBuffer->isUsingSyntaxHighlighting && shouldReLexBuffer)
	{
		currentBuffer->lexer.lex(point.line, false);
	}
}

void Frame::insertChar(char character)
{
	if (!warnIfBufferIsReadOnly()) return;
	
	Point startLocation = point;

	if (overwriteMode && point.col < currentBuffer->data[point.line].size())
	{
		currentBuffer->data[point.line][point.col] = character;
	}
	else
	{
		currentBuffer->data[point.line].insert(currentBuffer->data[point.line].begin() + point.col, character);
	}
	
	point.col += 1;
	point.targetCol = point.col;

	currentBuffer->addActionToUndoBuffer(Action::insertion(startLocation, point, std::string(1, character)));

	adjustOtherFramePointLocations(true, false);
	doCommonPointManipulationTasks();
	doCommonBufferManipulationTasks();
	if (shouldUpdatePopups) updatePopups();

	// Matching pairs
	// TODO(fkp): Check to see if there is already a matching character
	if (character == '{')
	{
		insertChar('}');
		movePointLeft();
	}
	if (character == '(')
	{
		insertChar(')');
		movePointLeft();
	}
	else if (character == '[')
	{
		insertChar(']');
		movePointLeft();
	}
}

void Frame::backspaceChar(unsigned int num, bool copyText)
{
	if (!warnIfBufferIsReadOnly()) return;
	
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
	if (shouldUpdatePopups) updatePopups();
}

void Frame::deleteChar(unsigned int num, bool copyText)
{
	if (!warnIfBufferIsReadOnly()) return;
	
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
	if (!warnIfBufferIsReadOnly()) return;

	// This is for proper expansion of braces
	bool isExpandingBraces = point.col > 0 &&
							 currentBuffer->data[point.line][point.col] == '}' &&
							 currentBuffer->data[point.line][point.col - 1] == '{';
	
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

	if (isExpandingBraces)
	{
		insertChar('\t');
		newLine();
		movePointLeft();
	}
}

void Frame::insertString(const std::string& string)
{
	if (!warnIfBufferIsReadOnly()) return;
	
	Point startLocation = point;
	bool oldShouldAddInformation = currentBuffer->shouldAddToUndoInformation;
	unsigned int oldTopLine = targetTopLine;
	currentBuffer->shouldAddToUndoInformation = false;

	for (char character : string)
	{
		if (character == '\n')
		{
			newLine();
		}
		else if (character != '\r') // Windows has CRLF endings
		{
			bool oldShouldReLexBuffer = shouldReLexBuffer;
			shouldReLexBuffer = false;
			
			insertChar(character);
			
			shouldReLexBuffer = oldShouldReLexBuffer;
		}
	}

	if (targetTopLine != oldTopLine)
	{
		centerPoint();
	}

	doCommonBufferManipulationTasks();
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
	Point startPoint = point;
	
	if (currentBuffer->type == BufferType::MiniBuffer)
	{
		// Should move to after the 'Execute: ' part
		point.col = Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' ') + 1;
	}
	else
	{
		point.col = 0;
	}

	unsigned int colMovedTo = point.col;
	
	while (currentBuffer->data[point.line][point.col] == ' ' ||
		   currentBuffer->data[point.line][point.col] == '\t')
	{
		point.col += 1;
	}

	if (point >= startPoint)
	{
		point.col = colMovedTo;
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
	unsigned int oldLineTop = targetTopLine;
	int newLineTop = (int) targetTopLine + numberOfLines;

	if (newLineTop > (int) currentBuffer->data.size() - 2)
	{
		newLineTop = (int) currentBuffer->data.size() - 2;
	}

	if (newLineTop < 0)
	{
		newLineTop = 0;
	}

	targetTopLine = newLineTop;;
	int numberOfLinesMoved = (int) targetTopLine - (int) oldLineTop;

	if (movePoint)
	{
		point.line += numberOfLinesMoved;

		if (point.line < 0)
		{
			point.line = 0;
		}

		if (point.line > (int) currentBuffer->data.size() - 2)
		{
			if (currentBuffer->data.size() - 2 > 0)
			{
				point.line = (int) currentBuffer->data.size() - 2;
			}
			else
			{
				point.line = 0;
			}
		}

		popupLines.clear();
		popupCurrentSuggestion = 0;
		moveColToTarget();
	}
}

void Frame::centerPoint()
{
	int numberOfLinesToMove = point.line - ((int) targetTopLine + (numberOfLinesInView / 2));
	moveView(numberOfLinesToMove, false);
}

#define WORD_SEPARATORS "`~!@#$%^&*()-=+[]{}\\|;:'\",.<>/?"

unsigned int Frame::findWordBoundaryLeft()
{
	static std::string wordSeparators = WORD_SEPARATORS;
	unsigned int numberOfChars = 0;
	Point currentLocation = point;
	currentLocation.buffer = currentBuffer;
	// If started on whitespace, stop at first non-whitespace
	// character. Otherwise, stop at first whitespace character.
	bool startedOnWhitespace;
	bool hasHitNonWhitespaceCharacter = false;

	if (isspace(currentBuffer->data[point.line][point.col]) ||
		(point.col > 0 && isspace(currentBuffer->data[point.line][point.col - 1])))
	{
		startedOnWhitespace = true;
	}
	else
	{
		startedOnWhitespace = false;
	}
	
	while (true)
	{
		currentLocation--;

		if (currentLocation.line == 0 && currentLocation.col == 0)
		{
			numberOfChars += 1;
			break;
		}
		else if (isspace(currentBuffer->data[currentLocation.line][currentLocation.col]) ||
				 currentLocation.col == currentBuffer->data[currentLocation.line].size())
		{
			if (hasHitNonWhitespaceCharacter)
			{
				break;
			}
		}
		else
		{
			hasHitNonWhitespaceCharacter = true;
		}

		if (numberOfChars != 0 &&
			wordSeparators.find(currentBuffer->data[currentLocation.line][currentLocation.col]) != std::string::npos)
		{
			break;
		}
		
		numberOfChars += 1;
	}

	return numberOfChars;
}

unsigned int Frame::findWordBoundaryRight()
{
	static std::string wordSeparators = WORD_SEPARATORS;
	unsigned int numberOfChars = 0;
	Point currentLocation = point;
	currentLocation.buffer = currentBuffer;
	// If started on whitespace, stop at first non-whitespace
	// character. Otherwise, stop at first whitespace character.
	bool startedOnWhitespace;

	if (point.col < currentBuffer->data[point.line].size() &&
		isspace(currentBuffer->data[point.line][point.col]) &&
		isspace(currentBuffer->data[point.line][point.col + 1]))
	{
		startedOnWhitespace = true;
	}
	else if (point.col == currentBuffer->data[point.line].size())
	{
		startedOnWhitespace = true;
	}
	else
	{
		startedOnWhitespace = false;
	}

	while (true)
	{
		numberOfChars += 1;
		currentLocation++;

		if (isspace(currentBuffer->data[currentLocation.line][currentLocation.col]))
		{
			if (!startedOnWhitespace)
			{
				break;
			}
		}
		else if (currentLocation.col == currentBuffer->data[currentLocation.line].size())
		{
			if (currentLocation != point || !currentLocation.isInBuffer())
			{
				break;
			}
		}
		else
		{
			if (startedOnWhitespace)
			{
				break;
			}
		}

		if (numberOfChars != 0 &&
			wordSeparators.find(currentBuffer->data[currentLocation.line][currentLocation.col]) != std::string::npos)
		{
			break;
		}
	}
	
	return numberOfChars;
}

void Frame::getRect(Font* currentFont, int* realPixelX, unsigned int* realPixelWidth, int* pixelX, int* pixelY, unsigned int* pixelWidth, unsigned int* pixelHeight)
{
	int tempRealPixelX = (int) (pcDimensions.x * windowWidth);
	unsigned int tempRealPixelWidth = (unsigned int) (pcDimensions.width * windowWidth);
	
	int tempPixelX = tempRealPixelX + (FRAME_BORDER_WIDTH * 2);
	int tempPixelY = (int) (pcDimensions.y * windowHeight);
	unsigned int tempPixelWidth = tempRealPixelWidth - (FRAME_BORDER_WIDTH * 2);
	unsigned int tempPixelHeight = (unsigned int) (pcDimensions.height * windowHeight);

	if (pcDimensions.y == 1.0f)
	{
		// This is the minibuffer
		tempPixelHeight = currentFont->size;
	}

	if (realPixelX) *realPixelX = tempRealPixelX;
	if (realPixelWidth) *realPixelWidth = tempRealPixelWidth;
	if (pixelX) *pixelX = tempPixelX;
	if (pixelY) *pixelY = tempPixelY;
	if (pixelWidth) *pixelWidth = tempPixelWidth;
	if (pixelHeight) *pixelHeight = tempPixelHeight;
}

void Frame::getPointRect(Font* currentFont, unsigned int tabWidth, int framePixelX, int framePixelY, float* pointX, float* pointY, float* pointWidth, float* pointHeight)
{
	float tempPointX = framePixelX;
	float tempPointY = framePixelY + ((point.line - currentTopLine) * currentFont->size);
	float tempPointWidth;
	float tempPointHeight = (float) currentFont->size;
	unsigned int numberOfColumnsInLine = 0;

	for (unsigned int i = 0; i < point.col; i++)
	{
		const Character& character = currentFont->chars[(unsigned char) currentBuffer->data[point.line][i]];

		if (currentBuffer->data[point.line][i] == '\n')
		{
			tempPointX = 0;
			tempPointY += currentFont->size;
			numberOfColumnsInLine = 0;
		}
		else if (currentBuffer->data[point.line][i] == '\t')
		{
			advanceToNextTabStop(tabWidth, currentFont, tempPointX, numberOfColumnsInLine);
		}
		else
		{
			tempPointX += character.advanceX;
			numberOfColumnsInLine += 1;
		}
	}

	if (point.col == currentBuffer->data[point.line].size())
	{
		tempPointWidth = (float) currentFont->maxGlyphAdvanceX;
	}
	else
	{
		unsigned char currentChar = currentBuffer->data[point.line][point.col];

		if (currentChar == '\n' || currentChar == '\t')
		{
			tempPointWidth = currentFont->chars[(unsigned char) ' '].advanceX;
		}
		else
		{
			tempPointWidth = currentFont->chars[currentChar].advanceX;
		}
	}

	if (pointX) *pointX = tempPointX;
	if (pointY) *pointY = tempPointY;
	if (pointWidth) *pointWidth = tempPointWidth;
	if (pointHeight) *pointHeight = tempPointHeight;
}

Token* Frame::getTokenUnderPoint(bool includeEnd)
{
	if (currentBuffer->isUsingSyntaxHighlighting &&
		point.line < currentBuffer->lexer.lineStates.size())
	{
		LineLexState& line = currentBuffer->lexer.lineStates[point.line];

		for (Token& token : line.tokens)
		{
			if (point.col >= token.start.col)
			{
				// There's probably a more compact way to do this, but
				// this is easier to read and comprehend.
				if (includeEnd)
				{
					if (point.col <= token.end.col)
					{
						return &token;
					}
				}
				else
				{
					if (point.col < token.end.col)
					{
						return &token;
					}
				}
			}
		}
	}

	return nullptr;
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
	if (!warnIfBufferIsReadOnly()) return;
	
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

bool Frame::warnIfBufferIsReadOnly()
{
	if (currentBuffer->isReadOnly)
	{
		writeToMinibuffer("Cannot modify buffer - read only.");
		return false;
	}

	return true;
}

// TODO(fkp): This method has a lot of code duplication, clean it up.
void Frame::updatePopups()
{
	if (!warnIfBufferIsReadOnly()) return;
	
	std::vector<std::pair<std::string::size_type, std::pair<std::string, std::string>>> foundMatches;
			
	if (currentBuffer->type == BufferType::MiniBuffer)
	{
		if (Commands::currentlyReading == MinibufferReading::Path)
		{
			// The path up to the last directory
			std::string::size_type startOfPathIndex = currentBuffer->data[0].find_first_of(' ') + 1;
			std::string::size_type lastSlashIndex = currentBuffer->data[0].find_last_of("/\\");
			std::string currentValidPath = currentBuffer->data[0].substr(startOfPathIndex, lastSlashIndex - startOfPathIndex + 1);

			if (lastSlashIndex == std::string::npos)
			{
				lastSlashIndex = startOfPathIndex - 1;
				currentValidPath = "";
			}
			
			std::string currentNextItem = currentBuffer->data[0].substr(lastSlashIndex + 1);

			if (std::filesystem::exists(currentValidPath))
			{
				// Iterates all filess and directories in the path
				for (const auto& file : std::filesystem::directory_iterator(currentValidPath))
				{
					std::string name = file.path().filename().string();
					std::error_code isDirectoryErrorCode;
				
					if (std::filesystem::is_directory(file.path(), isDirectoryErrorCode))
					{
						name += "/";
					}
				
					std::string::size_type index = name.find(currentNextItem);
				
					if (index != std::string::npos)
					{
						foundMatches.emplace_back(index, std::make_pair(name, ""));
					}
				}
			}
		}
		else if (Commands::currentlyReading == MinibufferReading::BufferName)
		{
			std::string bufferName = currentBuffer->data[0].substr(currentBuffer->data[0].find_first_of(' ') + 1);

			for (const std::pair<const std::string, Buffer*>& buffer : Buffer::buffersMap)
			{
				// This frame is the minibuffer
				if (buffer.second == currentBuffer)
				{
					continue;
				}
				
				std::string::size_type index = buffer.first.find(bufferName);

				if (index != std::string::npos)
				{
					foundMatches.emplace_back(index, std::make_pair(buffer.first, ""));
				}
			}
		}
		else if (Commands::currentlyReading == MinibufferReading::Command)
		{
			std::string commandText = currentBuffer->data[0].substr(currentBuffer->data[0].find_first_of(' ') + 1);

			if (commandText == "")
			{
				return;
			}

			for (const std::pair<const std::string, COMMAND_FUNC_SIG()>& command : Commands::nonEssentialCommandsMap)
			{
				std::string::size_type index = command.first.find(commandText);

				if (index != std::string::npos)
				{
					foundMatches.emplace_back(index, std::make_pair(command.first, ""));
				}
			}

			for (const std::pair<const std::string, COMMAND_FUNC_SIG()>& command : Commands::essentialCommandsMap)
			{
				std::string::size_type index = command.first.find(commandText);

				if (index != std::string::npos)
				{
					foundMatches.emplace_back(index, std::make_pair(command.first, ""));
				}
			}
		}
	}
	else
	{
		Token* tokenUnderPoint = getTokenUnderPoint(true);
	
		if (tokenUnderPoint &&
			point.col == tokenUnderPoint->end.col &&
			(tokenUnderPoint->type == Token::Type::IdentifierUsage ||
			 tokenUnderPoint->type == Token::Type::FunctionUsage ||
			 tokenUnderPoint->type == Token::Type::IdentifierDefinition ||
			 tokenUnderPoint->type == Token::Type::FunctionDefinition ||
			 tokenUnderPoint->type == Token::Type::TypeName ||
			 tokenUnderPoint->type == Token::Type::Keyword
			 /* tokenUnderPoint->type == Token::Type::PreprocessorDirective */))
		{
			std::string tokenText = currentBuffer->substrFromPoints(tokenUnderPoint->start, tokenUnderPoint->end);
		
			for (const std::pair<const std::string, std::string>& function : currentBuffer->functionDefinitions)
			{
				const std::string& functionName = function.first;

				if (tokenText != functionName)
				{
					std::string::size_type index = functionName.find(tokenText);

					if (index != std::string::npos)
					{
						foundMatches.emplace_back(index, std::make_pair(functionName, function.second));
					}
				}
			}

			// TODO(fkp): Should we really be iterating these every time?
			for (const std::string& keyword : Lexer::keywords)
			{
				if (tokenText != keyword)
				{
					std::string::size_type index = keyword.find(tokenText);
			
					if (index != std::string::npos)
					{
						foundMatches.emplace_back(index, std::make_pair(keyword, ""));
					}
				}
			}
		
			for (const std::string& type : Lexer::primitiveTypes)
			{
				if (tokenText != type)
				{
					std::string::size_type index = type.find(tokenText);
			
					if (index != std::string::npos)
					{
						foundMatches.emplace_back(index, std::make_pair(type, ""));
					}
				}
			}
		}
	}
	
	popupLines.clear();
	std::sort(foundMatches.begin(), foundMatches.end(), [](auto& left, auto& right)
														{
															return left.first < right.first;
														});

	for (std::pair<std::string::size_type, std::pair<std::string, std::string>>& match : foundMatches)
	{
		popupLines.emplace_back(std::move(match.second));
	}
}

void Frame::completeSuggestion()
{
	if (!warnIfBufferIsReadOnly()) return;
	
	if (popupLines.size() > 0)
	{
		bool oldShouldUpdatePopups = shouldUpdatePopups;
		shouldUpdatePopups = false;
		std::string suggestion = popupLines[popupCurrentSuggestion].first;
		
		Token* tokenUnderPoint = getTokenUnderPoint(true);
		Token token { Token::Type::Invalid, Point { 0, 0 } }; // Unused unless minibuffer

		// The getTokenUnderPoint() would not have worked as this
		// buffer is not lexed.
		if (currentBuffer->type == BufferType::MiniBuffer)
		{
			// NOTE(fkp): (Almost) copied from commands_definitions.inl
			std::string::size_type indexSlashBefore = currentBuffer->data[0].find_last_of("/\\", point.col - 1);
			std::string::size_type indexSlashAfter = currentBuffer->data[0].find_first_of("/\\", point.col);

			if (indexSlashBefore == std::string::npos)
			{
				indexSlashBefore = currentBuffer->data[0].find_first_of(" ");
			}

			if (indexSlashAfter == std::string::npos)
			{
				indexSlashAfter = currentBuffer->data[0].size();
			}

			token.start.col = (unsigned int) indexSlashBefore + 1;
			token.end.col = (unsigned int) indexSlashAfter;
			tokenUnderPoint = &token;
		}
		
		if (tokenUnderPoint)
		{
			point = tokenUnderPoint->end;

			while (point > tokenUnderPoint->start)
			{
				backspaceChar();
			}

			insertString(suggestion);
		}

		popupLines.clear();
		popupCurrentSuggestion = 0;
		shouldUpdatePopups = oldShouldUpdatePopups;
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
	if (!warnIfBufferIsReadOnly()) return;
	
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
	if (!warnIfBufferIsReadOnly()) return;
	
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
	if (!warnIfBufferIsReadOnly()) return;
	
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

void advanceToNextTabStop(unsigned int tabWidth, const Font* font, float& x, unsigned int& numberOfColumnsInLine)
{
	// TODO(fkp): This doesn't work with non-monopspaced fonts
	unsigned int numberOfColumnsToNextTabStop = tabWidth - (numberOfColumnsInLine % tabWidth);
	x += font->chars[(unsigned char) ' '].advanceX * numberOfColumnsToNextTabStop;
	numberOfColumnsInLine += numberOfColumnsToNextTabStop;
}
