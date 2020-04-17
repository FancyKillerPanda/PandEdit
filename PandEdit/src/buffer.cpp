//  ===== Date Created: 15 April, 2020 ===== 

#include "buffer.hpp"
#include "frame.hpp"

Buffer::Buffer(BufferType type)
	: type(type)
{
	// Makes sure there's at least one line in the buffer
	data.emplace_back();
}

void Buffer::doCommonPointManipulationTasks()
{
	pointFlashFrameCounter = 0;

	if (type != BufferType::MiniBuffer)
	{
		Frame::minibufferFrame->currentBuffer->data[0] = "";
		Frame::minibufferFrame->currentBuffer->col = 0;
	}
}

void Buffer::movePointLeft(unsigned int num)
{
	doCommonPointManipulationTasks();

	if (num == 0) num = 1;
	
	for (int i = 0; i < num; i++)
	{
		if (col > 0)
		{
			if (type == BufferType::MiniBuffer &&
				col <= Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' ') + 1)
			{
				// Should not be able to move into the 'Execute: ' part
				break;
			}
			
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

	targetCol = col;
}

void Buffer::movePointRight(unsigned int num)
{
	doCommonPointManipulationTasks();

	if (num == 0) num = 1;
	
	for (int i = 0; i < num; i++)
	{
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

	targetCol = col;
}

void Buffer::movePointUp()
{
	doCommonPointManipulationTasks();
	
	if (line > 0)
	{
		line -= 1;
		moveColToTarget();
	}
}

void Buffer::movePointDown()
{
	doCommonPointManipulationTasks();

	if (line < data.size() - 1)
	{
		line += 1;
		moveColToTarget();
	}
}

void Buffer::movePointHome()
{
	doCommonPointManipulationTasks();	

	if (type == BufferType::MiniBuffer)
	{
		// Should move to after the 'Execute: ' part
		col = Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' ') + 1;
	}
	else
	{
		col = 0;
	}
	
	targetCol = col;
}

void Buffer::movePointEnd()
{
	doCommonPointManipulationTasks();	

	col = data[line].size();
	targetCol = col;
}

void Buffer::insertChar(char character)
{
	doCommonPointManipulationTasks();

	data[line].insert(data[line].begin() + col, character);
	col += 1;
	targetCol = col;
}

void Buffer::backspaceChar(unsigned int num)
{
	doCommonPointManipulationTasks();

	if (num == 0) num = 1;
	
	for (int i = 0; i < num; i++)
	{
		if (col > 0)
		{
			if (type == BufferType::MiniBuffer &&
				col <= Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' ') + 1)
			{
				// Should not be able to backspace into the 'Execute: ' part
				break;
			}
			
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
	
	targetCol = col;
}

void Buffer::deleteChar(unsigned int num)
{
	doCommonPointManipulationTasks();
	
	if (num == 0) num = 1;
	
	for (int i = 0; i < num; i++)
	{
		if (col < data[line].size())
		{
			data[line].erase(col, 1);
		}
		else
		{
			if (line < data.size() - 1)
			{
				data[line] += data[line + 1];
				data.erase(data.begin() + line + 1);
			}
		}
	}
	
	targetCol = col;
}

void Buffer::newLine()
{
	doCommonPointManipulationTasks();
	
	std::string restOfLine { data[line].begin() + col, data[line].end() };
	data[line].erase(data[line].begin() + col, data[line].end());
	
	line += 1;
	col = 0;
	targetCol = col;

	data.insert(data.begin() + line, restOfLine);
}

unsigned int Buffer::findWordBoundaryLeft()
{
	unsigned int numberOfChars = 0;

	while (col - numberOfChars > 0)
	{
		// Stops at space, only if not the first character
		if (numberOfChars != 0 &&
			data[line][col - numberOfChars - 1] == ' ')
		{
			break;
		}

		numberOfChars += 1;
	}

	return numberOfChars;
}

unsigned int Buffer::findWordBoundaryRight()
{
	unsigned int numberOfChars = 0;

	while (col + numberOfChars < data[line].size())
	{
		// Stops at space, only if not the first character
		if (numberOfChars != 0 &&
			data[line][col + numberOfChars] == ' ')
		{
			break;
		}

		numberOfChars += 1;
	}

	return numberOfChars;
}

void Buffer::moveColToTarget()
{
	if (col > data[line].size())
	{
		col = data[line].size();
	}
	else
	{
		if (targetCol > data[line].size())
		{
			col = data[line].size();
		}
		else
		{
			col = targetCol;
		}
	}
}
