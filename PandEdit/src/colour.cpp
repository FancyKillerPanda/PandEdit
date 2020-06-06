//  ===== Date Created: 06 June, 2020 ===== 

#include "colour.hpp"

Colour normaliseColour(float r, float g, float b, float a)
{
	return { r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
}

Colour getDefaultTextColour()
{
	return normaliseColour(212, 212, 212, 255);
}

Colour getColourForTokenType(Token::Type type)
{
	switch (type)
	{
	case Token::Type::Number:					return normaliseColour(255, 174,   0, 255);
	case Token::Type::Character:				return normaliseColour(  0, 160,   9, 255);
	case Token::Type::String:					return normaliseColour(  0, 160,   9, 255);
	case Token::Type::EscapeSequence:			return normaliseColour( 62, 118, 202, 255);
	case Token::Type::IncludeAngleBracketPath:	return normaliseColour(  0, 160,   9, 255);
	case Token::Type::LineComment:				return normaliseColour(154, 154, 154, 255);
	case Token::Type::BlockComment:				return normaliseColour(154, 154, 154, 255);
	case Token::Type::Keyword:					return normaliseColour(184,   8, 180, 255);
	case Token::Type::PreprocessorDirective:	return normaliseColour(184,   8, 180, 255);
	case Token::Type::MacroName:				return normaliseColour(219, 219, 149, 255);
	case Token::Type::TypeName:					return normaliseColour( 62, 118, 202, 255);
	case Token::Type::IdentifierDefinition:		return normaliseColour(219, 219, 149, 255);
	case Token::Type::FunctionDefinition:		return normaliseColour(111, 169, 255, 255);

	default:									return getDefaultTextColour();
	}
}
