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
	  lastPoint(other.lastPoint),
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

		lastPoint = other.lastPoint;
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
		Frame::minibufferFrame->point.col = 0;
	}
}

void Buffer::movePointLeft(Frame& frame, unsigned int num)
{
	doCommonPointManipulationTasks();

	if (num == 0) num = 1;

	for (int i = 0; i < num; i++)
	{
		if (frame.point.col > 0)
		{
			if (type == BufferType::MiniBuffer &&
				frame.point.col <= Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' ') + 1)
			{
				// Should not be able to move into the 'Execute: ' part
				break;
			}

			frame.point.col -= 1;
		}
		else
		{
			if (frame.point.line > 0)
			{
				frame.point.line -= 1;
				frame.point.col = data[frame.point.line].size();
			}
		}
	}

	frame.point.targetCol = frame.point.col;
}

void Buffer::movePointRight(Frame& frame, unsigned int num)
{
	doCommonPointManipulationTasks();

	if (num == 0) num = 1;

	for (int i = 0; i < num; i++)
	{
		if (frame.point.col < data[frame.point.line].size())
		{
			frame.point.col += 1;
		}
		else
		{
			if (frame.point.line < data.size() - 1)
			{
				frame.point.line += 1;
				frame.point.col = 0;
			}
		}
	}

	frame.point.targetCol = frame.point.col;
}

void Buffer::movePointUp(Frame& frame)
{
	doCommonPointManipulationTasks();

	if (frame.point.line > 0)
	{
		frame.point.line -= 1;
		moveColToTarget(frame);
	}
}

void Buffer::movePointDown(Frame& frame)
{
	doCommonPointManipulationTasks();

	if (frame.point.line < data.size() - 1)
	{
		frame.point.line += 1;
		moveColToTarget(frame);
	}
}

void Buffer::movePointHome(Frame& frame)
{
	doCommonPointManipulationTasks();

	if (type == BufferType::MiniBuffer)
	{
		// Should move to after the 'Execute: ' part
		frame.point.col = Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' ') + 1;
	}
	else
	{
		frame.point.col = 0;
	}

	frame.point.targetCol = frame.point.col;
}

void Buffer::movePointEnd(Frame& frame)
{
	doCommonPointManipulationTasks();

	frame.point.col = data[frame.point.line].size();
	frame.point.targetCol = frame.point.col;
}

void Buffer::insertChar(Frame& frame, char character)
{
	doCommonPointManipulationTasks();

	data[frame.point.line].insert(data[frame.point.line].begin() + frame.point.col, character);
	frame.point.col += 1;
	frame.point.targetCol = frame.point.col;

	frame.adjustOtherFramePointLocations(true, false);
}

void Buffer::backspaceChar(Frame& frame, unsigned int num)
{
	doCommonPointManipulationTasks();

	if (num == 0) num = 1;

	for (int i = 0; i < num; i++)
	{
		if (frame.point.col > 0)
		{
			if (type == BufferType::MiniBuffer &&
				frame.point.col <= Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' ') + 1)
			{
				// Should not be able to backspace into the 'Execute: ' part
				break;
			}

			frame.point.col -= 1;
			data[frame.point.line].erase(frame.point.col, 1);

			frame.adjustOtherFramePointLocations(false, false);
		}
		else
		{
			if (frame.point.line > 0)
			{
				frame.point.line -= 1;
				frame.point.col = data[frame.point.line].size();

				data[frame.point.line] += data[frame.point.line + 1];
				data.erase(data.begin() + frame.point.line + 1);

				frame.adjustOtherFramePointLocations(false, true);
			}
		}
	}

	frame.point.targetCol = frame.point.col;
}

void Buffer::deleteChar(Frame& frame, unsigned int num)
{
	doCommonPointManipulationTasks();

	if (num == 0) num = 1;

	for (int i = 0; i < num; i++)
	{
		if (frame.point.col < data[frame.point.line].size())
		{
			data[frame.point.line].erase(frame.point.col, 1);
			frame.adjustOtherFramePointLocations(false, false);
		}
		else
		{
			if (frame.point.line < data.size() - 1)
			{
				data[frame.point.line] += data[frame.point.line + 1];
				data.erase(data.begin() + frame.point.line + 1);
				frame.adjustOtherFramePointLocations(false, true);
			}
		}
	}

	frame.point.targetCol = frame.point.col;
}

void Buffer::newLine(Frame& frame)
{
	doCommonPointManipulationTasks();

	std::string restOfLine { data[frame.point.line].begin() + frame.point.col, data[frame.point.line].end() };
	data[frame.point.line].erase(data[frame.point.line].begin() + frame.point.col, data[frame.point.line].end());

	frame.point.line += 1;
	frame.point.col = 0;
	frame.point.targetCol = frame.point.col;

	data.insert(data.begin() + frame.point.line, restOfLine);
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
	if (killRing.size() == 0 || textToCopy != killRing.back())
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
			frame.mark.line = frame.point.line;
			frame.mark.col = frame.point.col;
			
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
			frame.mark.line = frame.point.line;
			frame.mark.col = frame.point.col;
			
			insertString(frame, clipboardString);
			GlobalUnlock(clipboardData);

			killRing.push_back(std::string { clipboardString });
			killRingPointer = killRing.size() - 1;
		}
	}

	CloseClipboard();
}

void Buffer::pastePop(Frame& frame)
{
	frame.deleteTextPointToMark(false);
	killRingPointer -= 1;
	
	if (killRingPointer < 0)
	{
		killRingPointer = killRing.size() - 1;
		
		if (killRing.size() == 0)
		{
			return;
		}
	}
	
	paste(frame);
}

unsigned int Buffer::findWordBoundaryLeft(Frame& frame)
{
	unsigned int numberOfChars = 0;

	while (frame.point.col - numberOfChars > 0)
	{
		// Stops at space, only if not the first character
		if (numberOfChars != 0 &&
			data[frame.point.line][frame.point.col - numberOfChars - 1] == ' ')
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

	while (frame.point.col + numberOfChars < data[frame.point.line].size())
	{
		// Stops at space, only if not the first character
		if (numberOfChars != 0 &&
			data[frame.point.line][frame.point.col + numberOfChars] == ' ')
		{
			break;
		}

		numberOfChars += 1;
	}

	return numberOfChars;
}

void Buffer::moveColToTarget(Frame& frame)
{
	if (frame.point.col > data[frame.point.line].size())
	{
		frame.point.col = data[frame.point.line].size();
	}
	else
	{
		if (frame.point.targetCol > data[frame.point.line].size())
		{
			frame.point.col = data[frame.point.line].size();
		}
		else
		{
			frame.point.col = frame.point.targetCol;
		}
	}
}
