//  ===== Date Created: 04 May, 2020 ===== 

#if !defined(LEXER_HPP)
#define LEXER_HPP

#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

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
		String,
		Character,
		EscapeSequence,
		IncludeAngleBracketPath,
		LineComment,
		BlockComment,
		Keyword,
		PreprocessorDirective,
		MacroName,
		TypeName,
		IdentifierUsage,
		IdentifierDefinition,
		FunctionUsage,
		FunctionDefinition,

		// Operator tokens
		LeftParen,
		RightParen,
		LeftBrace,
		RightBrace,
		LeftBracket,
		RightBracket,

		Less,
		LessEqual,
		Greater,
		GreaterEqual,
		Equal,
		EqualEqual,
		Bang,
		BangEqual,
		Spaceship,

		BitAnd,
		BitAndEqual,
		LogicalAnd,
		BitOr,
		BitOrEqual,
		LogicalOr,
		BitXor,
		BitXorEqual,
		BitNot,
		ShiftLeft,
		ShiftLeftEqual,
		ShiftRight,
		ShiftRightEqual,

		Plus,
		PlusEqual,
		Minus,
		MinusEqual,
		Asterisk,
		AsteriskEqual,
		Slash,
		SlashEqual,
		Percent,
		PercentEqual,

		Increment,
		Decrement,

		Dot,
		Arrow,
		
		Comma,
		Question,
		Colon,
		Semicolon,
		ScopeResolution,

		Invalid
	};
	
public:
	Type type;
	// NOTE(fkp): This is not filled in for all token types.
	std::string data;
	
	Point start;
	Point end;

public:
	Token(Type type, Point start, std::string data = "")
		: type(type), start(start), data(data)
	{
	}

	Token(Type type, Point start, Point end, std::string data = "")
		: type(type), start(start), end(end), data(data)
	{
	}
};

struct LineLexState
{
public:
	enum class FinishType
	{
		Finished,
		UnendedString,
		UnendedComment,
	};
	
	std::vector<Token> tokens;
	FinishType finishType = FinishType::Finished;

public:
	Token& getTokenBefore(int index, bool excludeComments = false, bool excludeAsteriskAndAmpersand = false);
	Token& getTokenBefore(int index, int& numberOfTokensTravelled, bool excludeComments = false, bool excludeAsteriskAndAmpersand = false);
	Token& getTokenAtOrAfter(int index, bool excludeComments = false, bool excludeAsteriskAndAmpersand = false);
	Token& getTokenAtOrAfter(int index, int& numberOfTokensTravelled, bool excludeComments = false, bool excludeAsteriskAndAmpersand = false);
};

class Lexer
{
public:
	static std::unordered_set<std::string> keywords;
	static std::unordered_set<std::string> primitiveTypes;
	
	Buffer* buffer;
	std::vector<LineLexState> lineStates;
	
public:
	Lexer(Buffer* buffer);
	
	// TODO(fkp): Language of lexing
	void lex(unsigned int startLine, bool lexEntireBuffer);
	void addLine(Point splitPoint);
	void removeLine(Point newPoint);
	std::vector<Token> getTokens(unsigned int startLine, unsigned int endLine);

private:
	void lexString(Point& point, LineLexState::FinishType& currentLineLastFinishType);
	void lexCharacter(Point& point);
	void lexIncludePath(Point& point);
	void lexLineComment(Point& point);
	void lexBlockComment(Point& point, LineLexState::FinishType& currentLineLastFinishType);
	void lexNumber(Point& point);
	void lexPreprocessorDirective(Point& point);
	bool lexKeyword(const Point& startPoint, const Point& point, const std::string& tokenText);
	void lexIdentifier(const Point& startPoint, const Point& point, const std::string& tokenText);
	bool lexPunctuation(Point& point);

	void doFinalAdjustments();

	std::unordered_map<std::string, std::string> findFunctionsInBuffer();
	
	static bool isIdentifierStartCharacter(char character);
	static bool isIdentifierCharacter(char character);
	static bool isValidHexDigit(char character);
};

Colour normaliseColour(float r, float g, float b, float a);
Colour getDefaultTextColour();
Colour getColourForTokenType(Token::Type type); // TODO(fkp): This should have styling

#endif
