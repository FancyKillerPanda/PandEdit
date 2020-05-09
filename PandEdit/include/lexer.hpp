//  ===== Date Created: 04 May, 2020 ===== 

#if !defined(LEXER_HPP)
#define LEXER_HPP

#include <vector>
#include <string>
#include <array>

#include "point.hpp"

class Buffer;

struct Colour
{
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 0.0f;
};

class Token
{
public:
	enum class Type
	{
		Number,
		Character,
		String,
		LineComment,
		BlockComment,
		Keyword,
		PreprocessorDirective,
	};
	
public:
	static std::array<std::string, 88> keywords;
	
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
Colour getDefaultTextColour();
Colour getColourForTokenType(Token::Type type); // TODO(fkp): This should have styling

#endif
