//  ===== Date Created: 15 April, 2020 ===== 

#include "buffer.hpp"

Buffer::Buffer(BufferType type)
	: type(type)
{
	// Makes sure there's at least one line in the buffer
	data.emplace_back();
}

void Buffer::insertChar(char character)
{
	data[line].insert(data[line].begin() + col, character);
	col += 1;
}

void Buffer::backspaceChar()
{
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
	std::string restOfLine { data[line].begin() + col, data[line].end() };
	data[line].erase(data[line].begin() + col, data[line].end());
	
	line += 1;
	col = 0;

	data.insert(data.begin() + line, restOfLine);
}
