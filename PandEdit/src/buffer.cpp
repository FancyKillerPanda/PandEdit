//  ===== Date Created: 15 April, 2020 =====

#include <fstream>
#include <unordered_set>
#include <algorithm>

#include "buffer.hpp"
#include "frame.hpp"
#include "file_util.hpp"
#include "common.hpp"
#include "commands.hpp"

Buffer::Buffer(BufferType type, std::string name, std::string path)
	: type(type), name(name), path(path), lexer(this)
{
	if (path == "" || !doesFileExist(path.c_str()))
	{
		// Makes sure there's at least one line in the buffer
		data.emplace_back();
	}
	else
	{
		// This method does all the necessary initialisation of the data
		revertToFile();		
	}
	
	buffersMap.insert({ name, this });
}

Buffer::~Buffer()
{
	if (name == "*scratch*")
	{
		return;
	}
	
	buffersMap.erase(name);

	for (Frame* frame : *Frame::allFrames)
	{
		if (this == frame->currentBuffer)
		{
			// TODO(fkp): Go to previous
			// frame->currentBuffer = get("*scratch*");
			frame->switchToBuffer(get("*scratch*"));
		}
	}
}

Buffer::Buffer(Buffer&& other)
	: type(other.type), name(std::move(other.name)), data(std::move(other.data)),
	  lexer(other.lexer),
	  lastPoint(other.lastPoint), lastTopLine(other.lastTopLine)
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
		lastTopLine = other.lastTopLine;

		lexer = other.lexer;

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

void Buffer::addActionToUndoBuffer(Action&& action)
{
	if (!shouldAddToUndoInformation) return;
	
	if (undoInformationPointer < undoInformation.size())
	{
		undoInformation.erase(undoInformation.begin() + undoInformationPointer, undoInformation.end());
	}

	// Appending to the last insertion
	if (action.type == ActionType::Insertion &&
		undoInformation.size() > 0 && undoInformation.back().type == ActionType::Insertion &&
		action.data.size() == 1)
	{
		Action& lastAction = undoInformation.back();

		// TODO(fkp): This is is a little unwieldy
		if (lastAction.end.line == lastAction.start.line &&
			lastAction.end.col - lastAction.start.col < 16 &&
			lastAction.end.col > 0 && lastAction.end.col <= data[lastAction.end.line].size() &&
			data[lastAction.end.line][lastAction.end.col - 1] != ' ' &&
			data[lastAction.end.line][lastAction.end.col - 1] != '\t' &&
			lastAction.end.col - 1 != data[lastAction.end.line].size())
		{
			if (action.data[0] == '\n')
			{
				lastAction.end.line += 1;
				lastAction.end.col = 0;
			}
			else
			{
				lastAction.end.col += 1;
			}
			
			lastAction.data += action.data;
			return;
		}
	}

	if (undoInformation.size() >= 64)
	{
		undoInformation.pop_front();
	}
	
	undoInformation.push_back(std::move(action));
	undoInformationPointer = undoInformation.size();
	numberOfActionsSinceSave += 1;
}

bool Buffer::undo(Frame& frame)
{
	if (undoInformationPointer == 0 || undoInformation.size() == 0)
	{
		return false;
	}

	shouldAddToUndoInformation = false;
	undoInformationPointer -= 1;
	numberOfActionsSinceSave -= 1;
	const Action& action = undoInformation[undoInformationPointer];

	switch (action.type)
	{
	case ActionType::Insertion:
	{
		frame.point = action.end;

		while (frame.point > action.start)
		{
			frame.backspaceChar();
		}
	} break;

	case ActionType::Deletion:
	{
		frame.point = action.start;
		frame.insertString(action.data);
		frame.point = action.end;
	} break;
	}

	shouldAddToUndoInformation = true;
	return true;
}

// NOTE(fkp): This is basically the exact opposite to undo()
bool Buffer::redo(Frame& frame)
{
	if (undoInformationPointer == undoInformation.size())
	{
		return false;
	}

	shouldAddToUndoInformation = false;
	const Action& action = undoInformation[undoInformationPointer];

	switch (action.type)
	{
	case ActionType::Insertion:
	{
		frame.point = action.start;
		frame.insertString(action.data);
		frame.point = action.end;
	} break;

	case ActionType::Deletion:
	{
		frame.point = action.end;

		while (frame.point > action.start)
		{
			frame.backspaceChar();
		}
	} break;
	}
	
	undoInformationPointer += 1;
	numberOfActionsSinceSave += 1;
	shouldAddToUndoInformation = true;

	return true;
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

	numberOfActionsSinceSave = 0;
	printf("Info: Saved buffer to file '%s'.\n", path.c_str());
}

void Buffer::revertToFile()
{
	if (path == "")
	{
		writeToMinibuffer("Error: No file associated with this buffer.");
		return;
	}
	else if (!doesFileExist(path.c_str()))
	{
		writeToMinibuffer("Error: File no longer exists on disk.");
		return;
	}

	data.clear();
	std::string fileContents = readFile(path.c_str());

	std::string::size_type pos = 0;
	std::string::size_type previous = 0;

	while ((pos = fileContents.find("\n", previous)) != std::string::npos)
	{
		data.emplace_back(fileContents.substr(previous, pos - previous));
		previous = pos + 1;
	}

	// Last one
	data.emplace_back(fileContents.substr(previous));

	// Adjustment of the point and mark in relevant frames
	for (Frame* frame : *Frame::allFrames)
	{
		if (frame->currentBuffer != this)
		{
			continue;
		}
		
		if (frame->point.line >= data.size())
		{
			frame->point.line = data.size();
		}

		if (frame->point.col > data[frame->point.line].size())
		{
			frame->point.col = data[frame->point.line].size();
		}

		frame->mark.line = frame->point.line;
		frame->mark.col = frame->point.col;
	}

	// TODO(fkp): Maybe make it so undo history isn't cleared?
	undoInformation.clear();
	undoInformationPointer = 0;
	numberOfActionsSinceSave = 0;
	
	// Lexing
	// Automatic syntax highlighting based on file extension
	// TODO(fkp): Different highlighting for C files
	std::string extension = path.substr(path.find_last_of('.') + 1);
	std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return std::tolower(c); });
		
	static std::unordered_set<std::string> cppExtensions = {
		"h", "hpp", "hxx",
		"c", "cpp", "cxx", "cc",
	};

	if (cppExtensions.find(extension) != cppExtensions.end())
	{
		isUsingSyntaxHighlighting = true;
		lexer.lex(0, true);
	}
}

std::string Buffer::substrFromPoints(const Point& start, const Point& end)
{
	std::string substr = "";

	for (int i = start.line; i <= end.line; i++)
	{
		if (i < 0 || i >= data.size())
		{
			break;
		}
		
		substr += data[i];
		substr += std::string(1, '\n');
	}

	return ::substrFromPoints(substr, start, end, start.line);
}

std::string substrFromPoints(const std::string& string, const Point& start, const Point& end, unsigned int lineOffset)
{
	if (start > end)
	{
		ERROR_ONCE("Error: Start cannot be after end in substr.\n");
		return "";
	}

	if (lineOffset > start.line)
	{
		ERROR_ONCE("Error: String starts after the requested start point in substr.\n");
		return "";
	}
	
	unsigned int startIndex = 0;
	unsigned int currentIndex = 0;
	unsigned int linesPassed = lineOffset;
	
	while (currentIndex < string.size() && linesPassed < start.line)
	{
		if (string[currentIndex] == '\n')
		{
			linesPassed += 1;
		}

		currentIndex += 1;
	}

	currentIndex += start.col;
	startIndex = currentIndex;
	linesPassed = 0;
	unsigned int length = 0;

	while (currentIndex < string.size() && linesPassed < end.line - start.line)
	{
		if (string[currentIndex] == '\n')
		{
			linesPassed += 1;
		}

		length += 1;
		currentIndex += 1;
	}

	// The final line
	if (start.line == end.line)
	{
		length += end.col - start.col;
	}
	else
	{
		length += end.col;
	}

	return string.substr(startIndex, length);
}

Point getPointAtEndOfString(const std::string& string, unsigned int lineOffset)
{
	Point result;
	result.line = lineOffset;

	for (const char& character : string)
	{
		if (character == '\n')
		{
			result.line += 1;
			result.col = 0;
		}
		else
		{
			result.col += 1;
		}
	}

	return result;
}
