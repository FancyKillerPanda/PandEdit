//  ===== Date Created: 28 April, 2020 ===== 

#if !defined(POINT_HPP)
#define POINT_HPP

class Buffer;

class Point
{
public:
	unsigned int line = 0;
	unsigned int col = 0;
	unsigned int targetCol = 0;

	const Buffer* buffer = nullptr;
	
public:
	Point(const Buffer* buffer = nullptr);
	Point(unsigned int line, unsigned int col, const Buffer* buffer = nullptr);
	
	bool operator==(const Point& other) const;
	bool operator!=(const Point& other) const;
	
	bool operator<(const Point& other) const;
	bool operator>(const Point& other) const;
	bool operator<=(const Point& other) const;
	bool operator>=(const Point& other) const;

	Point& operator++();
	Point& operator--();
	Point operator++(int);
	Point operator--(int);

	bool isInBuffer();
	Point& moveNext(bool force = false);
	Point& movePrevious();
	unsigned int distanceTo(const Point& other) const;
};

#endif
