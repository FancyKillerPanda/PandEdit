//  ===== Date Created: 15 April, 2020 =====

#include <windows.h>

#include "buffer.hpp"
#include "frame.hpp"

std::unordered_map<std::string, Buffer*> Buffer::buffersMap;
std::vector<std::string> Buffer::killRing;
int Buffer::killRingPointer = -1;
DWORD Buffer::lastClipboardSequenceNumber = 0;

Buffer::Buffer(BufferType type, std::string name)
	: type(type), name(name)
{
	// Makes sure there's at least one line in the buffer
	data.emplace_back();

	buffersMap.insert({ name, this });
}

Buffer::~Buffer()
{
	buffersMap.erase(name);

	for (Frame& frame : *Frame::allFrames)
	{
		if (this == frame.currentBuffer)
		{
			// TODO(fkp): Go to previous
			frame.currentBuffer = get("*scratch*");
		}
	}
}

Buffer::Buffer(Buffer&& other)
	: type(other.type), name(std::move(other.name)), data(std::move(other.data)),
	  lastLine(other.lastLine), lastCol(other.lastCol), lastTargetCol(other.lastTargetCol),
	  pointFlashFrameCounter(other.pointFlashFrameCounter)
{
	buffersMap[name] = this;
	other.name = "";
}

Buffer& Buffer::operator=(Buffer&& other)
{
	if (this != &other)
	{
		type = other.type;
		name = std::move(other.name);
		data = std::move(other.data);

		lastLine = other.lastLine;
		lastCol = other.lastCol;
		lastTargetCol = other.lastTargetCol;
		pointFlashFrameCounter = other.pointFlashFrameCounter;

		buffersMap[name] = this;
		other.name = "";
	}

	return *this;
}

Buffer* Buffer::get(const std::string& name)
{
	auto result = buffersMap.find(name);

	if (result != buffersMap.end())
	{
		return result->second;
	}
	else
	{
		return nullptr;
	}
}

void Buffer::doCommonPointManipulationTasks()
{
	pointFlashFrameCounter = 0;

	if (type != BufferType::MiniBuffer)
	{
		Frame::minibufferFrame->currentBuffer->data[0] = "";
		Frame::minibufferFrame->col = 0;
	}
}

void Buffer::movePointLeft(Frame& frame, unsigned int num)
{
	doCommonPointManipulationTasks();

	if (num == 0) num = 1;

	for (int i = 0; i < num; i++)
	{
		if (frame.col > 0)
		{
			if (type == BufferType::MiniBuffer &&
				frame.col <= Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' ') + 1)
			{
				// Should not be able to move into the 'Execute: ' part
				break;
			}

			frame.col -= 1;
		}
		else
		{
			if (frame.line > 0)
			{
				frame.line -= 1;
				frame.col = data[frame.line].size();
			}
		}
	}

	frame.targetCol = frame.col;
}

void Buffer::movePointRight(Frame& frame, unsigned int num)
{
	doCommonPointManipulationTasks();

	if (num == 0) num = 1;

	for (int i = 0; i < num; i++)
	{
		if (frame.col < data[frame.line].size())
		{
			frame.col += 1;
		}
		else
		{
			if (frame.line < data.size() - 1)
			{
				frame.line += 1;
				frame.col = 0;
			}
		}
	}

	frame.targetCol = frame.col;
}

void Buffer::movePointUp(Frame& frame)
{
	doCommonPointManipulationTasks();

	if (frame.line > 0)
	{
		frame.line -= 1;
		moveColToTarget(frame);
	}
}

void Buffer::movePointDown(Frame& frame)
{
	doCommonPointManipulationTasks();

	if (frame.line < data.size() - 1)
	{
		frame.line += 1;
		moveColToTarget(frame);
	}
}

void Buffer::movePointHome(Frame& frame)
{
	doCommonPointManipulationTasks();

	if (type == BufferType::MiniBuffer)
	{
		// Should move to after the 'Execute: ' part
		frame.col = Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' ') + 1;
	}
	else
	{
		frame.col = 0;
	}

	frame.targetCol = frame.col;
}

void Buffer::movePointEnd(Frame& frame)
{
	doCommonPointManipulationTasks();

	frame.col = data[frame.line].size();
	frame.targetCol = frame.col;
}

void Buffer::insertChar(Frame& frame, char character)
{
	doCommonPointManipulationTasks();

	data[frame.line].insert(data[frame.line].begin() + frame.col, character);
	frame.col += 1;
	frame.targetCol = frame.col;

	frame.adjustOtherFramePointLocations(true, false);
}

void Buffer::backspaceChar(Frame& frame, unsigned int num)
{
	doCommonPointManipulationTasks();

	if (num == 0) num = 1;

	for (int i = 0; i < num; i++)
	{
		if (frame.col > 0)
		{
			if (type == BufferType::MiniBuffer &&
				frame.col <= Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' ') + 1)
			{
				// Should not be able to backspace into the 'Execute: ' part
				break;
			}

			frame.col -= 1;
			data[frame.line].erase(frame.col, 1);

			frame.adjustOtherFramePointLocations(false, false);
		}
		else
		{
			if (frame.line > 0)
			{
				frame.line -= 1;
				frame.col = data[frame.line].size();

				data[frame.line] += data[frame.line + 1];
				data.erase(data.begin() + frame.line + 1);

				frame.adjustOtherFramePointLocations(false, true);
			}
		}
	}

	frame.targetCol = frame.col;
}

void Buffer::deleteChar(Frame& frame, unsigned int num)
{
	doCommonPointManipulationTasks();

	if (num == 0) num = 1;

	for (int i = 0; i < num; i++)
	{
		if (frame.col < data[frame.line].size())
		{
			data[frame.line].erase(frame.col, 1);
			frame.adjustOtherFramePointLocations(false, false);
		}
		else
		{
			if (frame.line < data.size() - 1)
			{
				data[frame.line] += data[frame.line + 1];
				data.erase(data.begin() + frame.line + 1);
				frame.adjustOtherFramePointLocations(false, true);
			}
		}
	}

	frame.targetCol = frame.col;
}

void Buffer::newLine(Frame& frame)
{
	doCommonPointManipulationTasks();

	std::string restOfLine { data[frame.line].begin() + frame.col, data[frame.line].end() };
	data[frame.line].erase(data[frame.line].begin() + frame.col, data[frame.line].end());

	frame.line += 1;
	frame.col = 0;
	frame.targetCol = frame.col;

	data.insert(data.begin() + frame.line, restOfLine);
	frame.adjustOtherFramePointLocations(true, true);
}

void Buffer::insertString(Frame& frame, const std::string& string)
{
	for (char character : string)
	{
		if (character == '\n')
		{
			newLine(frame);
		}
		else if (character != '\r') // Windows has CRLF endings
		{
			insertChar(frame, character);
		}
	}
}

void Buffer::copyRegion(Frame& frame)
{
	std::string textToCopy = frame.getTextPointToMark();
	
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
	if (textToCopy != killRing.back())
	{
		killRing.push_back(std::move(textToCopy));
		killRingPointer = killRing.size() - 1;
	}
	
	lastClipboardSequenceNumber = GetClipboardSequenceNumber();
}

void Buffer::paste(Frame& frame)
{
	if (GetClipboardSequenceNumber() != lastClipboardSequenceNumber)
	{
		pasteClipboard(frame);
		
	}
	else
	{
		// TODO(fkp): Use the kill ring pointer
		if (killRing.size() > 0)
		{
			insertString(frame, killRing[killRingPointer]);
		}
	}
}

void Buffer::pasteClipboard(Frame& frame)
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
			insertString(frame, clipboardString);
			GlobalUnlock(clipboardData);

			killRing.push_back(std::string { clipboardString });
			killRingPointer = killRing.size() - 1;
		}
	}

	CloseClipboard();
}

unsigned int Buffer::findWordBoundaryLeft(Frame& frame)
{
	unsigned int numberOfChars = 0;

	while (frame.col - numberOfChars > 0)
	{
		// Stops at space, only if not the first character
		if (numberOfChars != 0 &&
			data[frame.line][frame.col - numberOfChars - 1] == ' ')
		{
			break;
		}

		numberOfChars += 1;
	}

	return numberOfChars;
}

unsigned int Buffer::findWordBoundaryRight(Frame& frame)
{
	unsigned int numberOfChars = 0;

	while (frame.col + numberOfChars < data[frame.line].size())
	{
		// Stops at space, only if not the first character
		if (numberOfChars != 0 &&
			data[frame.line][frame.col + numberOfChars] == ' ')
		{
			break;
		}

		numberOfChars += 1;
	}

	return numberOfChars;
}

void Buffer::moveColToTarget(Frame& frame)
{
	if (frame.col > data[frame.line].size())
	{
		frame.col = data[frame.line].size();
	}
	else
	{
		if (frame.targetCol > data[frame.line].size())
		{
			frame.col = data[frame.line].size();
		}
		else
		{
			frame.col = frame.targetCol;
		}
	}
}
