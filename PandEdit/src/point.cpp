//  ===== Date Created: 28 April, 2020 ===== 

#include "point.hpp"
#include "buffer.hpp"
#include "frame.hpp"
#include "common.hpp"

Point::Point(const Buffer* buffer)
	: buffer(buffer)
{
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
	if (!buffer)
	{
		ERROR_ONCE("Error: Cannot use point++, no buffer provided.\n");
		return *this;
	}
	
	if (col < buffer->data[line].size())
	{
		col += 1;
	}
	else
	{
		if (line < buffer->data.size() - 1)
		{
			line += 1;
			col = 0;
		}
	}

	return *this;
}

Point& Point::operator--()
{
	if (!buffer)
	{
		ERROR_ONCE("Error: Cannot use point--, no buffer provided.\n");
		return *this;
	}

	if (col > 0)
	{
		if (buffer->type == BufferType::MiniBuffer &&
			col <= Frame::minibufferFrame->currentBuffer->data[0].find_first_of(' ') + 1)
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
