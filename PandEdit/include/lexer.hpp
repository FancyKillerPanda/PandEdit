//  ===== Date Created: 04 May, 2020 ===== 

#if !defined(LEXER_HPP)
#define LEXER_HPP

#include <vector>
#include <string>
#include <unordered_set>

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
		IncludeAngleBracketPath,
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

struct LineLexState
{
	enum class FinishType
	{
		Finished,
		UnendedString,
		UnendedComment,
	};
	
	std::vector<Token> tokens;
	FinishType finishType = FinishType::Finished;
};

class Lexer
{
public:
	static std::unordered_set<std::string> keywords;
	
	Buffer* buffer;
	std::vector<LineLexState> lineStates;
	
public:
	Lexer(Buffer* buffer);
	
	// TODO(fkp): Language of lexing
	void lex(unsigned int startLine);
	std::vector<Token> getTokens(unsigned int startLine, unsigned int endLine);

private:
	static bool isIdentifierStartCharacter(char character);
	static bool isIdentifierCharacter(char character);
	static bool isValidDigit(char character);
};

Colour normaliseColour(float r, float g, float b, float a);
Colour getDefaultTextColour();
Colour getColourForTokenType(Token::Type type); // TODO(fkp): This should have styling

#endif
