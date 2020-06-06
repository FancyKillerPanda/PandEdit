//  ===== Date Created: 06 June, 2020 ===== 

#if !defined(TOKEN_HPP)
#define TOKEN_HPP

#include <string>
#include "point.hpp"

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
	Token(Type type, Point start, std::string data = "");
	Token(Type type, Point start, Point end, std::string data = "");
};

#endif
