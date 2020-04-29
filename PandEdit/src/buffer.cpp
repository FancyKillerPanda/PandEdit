//  ===== Date Created: 15 April, 2020 =====

#include <windows.h>
#include <fstream>

#include "buffer.hpp"
#include "frame.hpp"
#include "file_util.hpp"

std::unordered_map<std::string, Buffer*> Buffer::buffersMap;
std::vector<std::string> Buffer::killRing;
int Buffer::killRingPointer = -1;
DWORD Buffer::lastClipboardSequenceNumber = 0;

Buffer::Buffer(BufferType type, std::string name, std::string path)
	: type(type), name(name), path(path)
{
	if (path == "")
	{
		// Makes sure there's at least one line in the buffer
		data.emplace_back();
	}
	else
	{
		std::string fileContents = readFile(path.c_str(), true);

		std::string::size_type pos = 0;
		std::string::size_type previous = 0;

		while ((pos = fileContents.find("\n", previous)) != std::string::npos)
		{
			data.emplace_back(std::move(fileContents.substr(previous, pos - previous)));
			previous = pos + 1;
		}

		// Last one
		data.emplace_back(std::move(fileContents.substr(previous)));
	}
	
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
	  lastPoint(other.lastPoint)
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

Buffer* Buffer::getFromFilePath(const std::string& path)
{
	// TODO(fkp): Iterating an unordered_map is really bad.
	for (std::pair<const std::string, Buffer*>& pair : buffersMap)
	{
		if (pair.second->path == path)
		{
			return pair.second;
		}
	}

	return nullptr;
}

void Buffer::saveToFile()
{
	// TODO(fkp): Check if changes need to be saved
	if (path == "")
	{
		printf("Error: Cannot save non-file-visiting buffer.\n");
		return;
	}

	std::ofstream file(path, std::ios::trunc);

	if (!file)
	{
		printf("Error: Failed to open file '%s' for saving to.\n", path.c_str());
		return;
	}

	for (const std::string& line : data)
	{
		file << line << '\n';
	}

	printf("Info: Saved buffer to file '%s'.\n", path.c_str());
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
			
			frame.insertString(killRing[killRingPointer]);
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
			
			frame.insertString(clipboardString);
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
