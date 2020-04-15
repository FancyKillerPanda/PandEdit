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
