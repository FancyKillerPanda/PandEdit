//  ===== Date Created: 04 May, 2020 ===== 

#if !defined(LEXER_HPP)
#define LEXER_HPP

#include <vector>
#include "point.hpp"

class Buffer;

class Token
{
public:
	enum class Type
	{
		Number,
		Character,
		String,
	};
	
public:
	Type type;
	Point start;
	Point end;

public:
	Token(Type type, Point start)
		: type(type), start(start)
	{
	}

	Token(Type type, Point start, Point end)
		: type(type), start(start), end(end)
	{
	}
};

void lexCppBuffer(Buffer* buffer);

#endif
