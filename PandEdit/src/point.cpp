//  ===== Date Created: 28 April, 2020 ===== 

#include "point.hpp"
#include "buffer.hpp"
#include "frame.hpp"
#include "common.hpp"

Point::Point(const Buffer* buffer)
	: buffer(buffer)
{
}

Point::Point(unsigned int line, unsigned int col, const Buffer* buffer)
	: line(line), col(col), buffer(buffer)
{
}

Point Point::operator+(int number)
{
	Point result = *this;

	for (int i = 0; i < number; i++)
	{
		result++;
	}

	return result;
}

Point Point::operator-(int number)
{
	Point result = *this;

	for (int i = 0; i < number; i++)
	{
		result--;
	}

	return result;
}

bool Point::operator==(const Point& other) const
{
	return (line == other.line) && (col == other.col) && (targetCol == other.targetCol);
}

bool Point::operator!=(const Point& other) const
{
	return !(*this == other);
}

bool Point::operator<(const Point& other) const
{
	if (line < other.line)
	{
		return true;
	}
	else if (line == other.line)
	{
		if (col < other.col)
		{
			return true;
		}
	}

	return false;
}

bool Point::operator>(const Point& other) const
{
	return other < *this;
}

bool Point::operator<=(const Point& other) const
{
	return (*this < other) || (*this == other);
}

bool Point::operator>=(const Point& other) const
{
	return !(*this < other);
}

Point& Point::operator++()
{
	return moveNext();
}

Point& Point::operator--()
{
	return movePrevious();
}

Point Point::operator++(int)
{
	Point temp = *this;
	++*this;
	return temp;
}

Point Point::operator--(int)
{
	Point temp = *this;
	--*this;
	return temp;
}

bool Point::isInBuffer()
{
	if (!buffer)
	{
		ERROR_ONCE("Error: Cannot use Point::isInBuffer(), no buffer provided.\n");
		return false;		
	}

	if (line >= buffer->data.size())
	{
		return false;
	}

	if (col > buffer->data[line].size())
	{
		return false;
	}

	return true;
}

Point& Point::moveNext(bool force)
{
	if (!buffer)
	{
		ERROR_ONCE("Error: Cannot use point++, no buffer provided.\n");
		return *this;
	}

	if (!isInBuffer())
	{
		return *this;
	}
	
	if (col < buffer->data[line].size())
	{
		col += 1;
	}
	else
	{
		if (line < buffer->data.size() - 1 || force)
		{
			line += 1;
			col = 0;
		}
	}

	return *this;
}

Point& Point::movePrevious()
{
	if (!buffer)
	{
		ERROR_ONCE("Error: Cannot use point--, no buffer provided.\n");
		return *this;
	}

	if (col > 0)
	{
		if (buffer->type == BufferType::MiniBuffer &&
			col <= Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' '))
		{
			// Should not be able to move into the 'Execute: ' part
			return *this;
		}

		col -= 1;
	}
	else
	{
		if (line > 0)
		{
			line -= 1;
			col = buffer->data[line].size();
		}
	}

	return *this;
}

unsigned int Point::distanceTo(const Point& other) const
{
	int count = 0;
	Point location = *this;
	
	while (location < other)
	{
		count += 1;
		location++;
	}

	while (location > other)
	{
		count += 1;
		location--;
	}

	return count;
}
