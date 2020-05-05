//  ===== Date Created: 15 April, 2020 =====

#include <fstream>

#include "buffer.hpp"
#include "frame.hpp"
#include "file_util.hpp"

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
}

bool Buffer::undo(Frame& frame)
{
	if (undoInformationPointer == 0 || undoInformation.size() == 0)
	{
		return false;
	}

	shouldAddToUndoInformation = false;
	undoInformationPointer -= 1;
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

	printf("Info: Saved buffer to file '%s'.\n", path.c_str());
}
