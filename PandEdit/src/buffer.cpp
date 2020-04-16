//  ===== Date Created: 15 April, 2020 ===== 

#include "buffer.hpp"

Buffer::Buffer(BufferType type)
	: type(type)
{
	// Makes sure there's at least one line in the buffer
	data.emplace_back();
}

void Buffer::movePointLeft()
{
	pointFlashFrameCounter = 0;
	
	if (col > 0)
	{
		col -= 1;
	}
	else
	{
		if (line > 0)
		{
			line -= 1;
			col = data[line].size();
		}
	}
}

void Buffer::movePointRight()
{
	pointFlashFrameCounter = 0;

	if (col < data[line].size())
	{
		col += 1;
	}
	else
	{
		if (line < data.size() - 1)
		{
			line += 1;
			col = 0;
		}
	}
}

void Buffer::movePointUp()
{
	pointFlashFrameCounter = 0;

	if (line > 0)
	{
		line -= 1;

		if (col > data[line].size())
		{
			col = data[line].size();
		}
	}
}

void Buffer::movePointDown()
{
	pointFlashFrameCounter = 0;

	if (line < data.size() - 1)
	{
		line += 1;

		if (col > data[line].size())
		{
			col = data[line].size();
		}
	}
}

void Buffer::insertChar(char character)
{
	pointFlashFrameCounter = 0;

	data[line].insert(data[line].begin() + col, character);
	col += 1;
}

void Buffer::backspaceChar()
{
	pointFlashFrameCounter = 0;

	if (col > 0)
	{
		col -= 1;
		data[line].erase(col, 1);
	}
	else
	{
		if (line > 0)
		{
			line -= 1;
			col = data[line].size();

			data[line] += data[line + 1];
			data.erase(data.begin() + line + 1);
		}
	}
}

void Buffer::newLine()
{
	pointFlashFrameCounter = 0;

	std::string restOfLine { data[line].begin() + col, data[line].end() };
	data[line].erase(data[line].begin() + col, data[line].end());
	
	line += 1;
	col = 0;

	data.insert(data.begin() + line, restOfLine);
}
