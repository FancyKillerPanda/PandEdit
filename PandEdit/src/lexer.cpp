//  ===== Date Created: 04 May, 2020 ===== 

#include "lexer.hpp"
#include "buffer.hpp"

// 1. NOTE(fkp): These are only keywords in some contexts
std::array<std::string, 88> Token::keywords = {
	"alignas", "alignof", "sizeof", "typeid", "decltype",
	
	"and", "and_eq", "bitand", "bitor", "compl",
	"not", "not_eq", "or", "or_eq", "xor", "xor_eq",
	
	"atomic_cancel", "atomic_commit", "atomic_noexcept",

	"break", "case", "continue", "default", "do", "else",
	"for", "goto", "if", "return", "switch", "while",

	"const", "consteval", "constexpr", "constinit", "const_cast",

	"false", "nullptr", "true", "void",
	/* "bool", "char", "char8_t", "char16_t", "char32_t",
	"double", "float", "int", "long", "short",
	"signed", "unsigned", "wchar_t", */

	"auto", "class", "delete", "enum", "explicit", "final" /* note 1 */,
	"friend", "inline", "mutable", "namespace", "new", "noexcept",
	"operator", "override" /* note 1 */, "private", "protected",
	"public", "struct", "template", "this", "typedef", "typename",
	"union", "using", "virtual", "volatile",

	"catch", "throw", "try",
	
	"co_await", "co_return", "co_yield", "synchronized", "thread_local",

	"concept", "export", "import" /* note 1 */,
	"module" /* note 1 */, "requires",
	
	"extern", "register", "static",

	"dynamic_cast", "reinterpret_cast", "static_assert", "static_cast",
	
	"asm", "reflexpr",
};

bool isIdentifierStartCharacter(char character)
{
	bool lowercaseAToZ = character >= 'a' && character <= 'z';
	bool uppercaseAToZ = character >= 'A' && character <= 'Z';
	bool underscore = character == '_';
	
	return lowercaseAToZ || uppercaseAToZ || underscore;
}

bool isIdentifierCharacter(char character)
{
	bool number = character >= '0' && character <= '9';
	return isIdentifierStartCharacter(character) || number;
}

void lexCppBuffer(Buffer* buffer)
{
	// Empty buffer
	if (buffer->data.size() == 0 ||
		(buffer->data.size() == 1 && buffer->data[0].size() == 0))
	{
		return;
	}

	Point point { buffer };
	buffer->tokens.clear();

	while (point.isInBuffer())
	{
		char character = buffer->data[point.line][point.col];
		
		switch (character)
		{
		case '\'':
		{
			Token token { Token::Type::Character, point };
			
			do
			{
				point.moveNext(true);
			} while (point.isInBuffer() && buffer->data[point.line][point.col] != '\'');

			// To go over the last quote
			point.moveNext(true);

			token.end = point;
			buffer->tokens.push_back(token);
		} break;
			
		case '"':
		{
			Token token { Token::Type::String, point };
			
			do
			{
				point.moveNext(true);
			} while (point.isInBuffer() && buffer->data[point.line][point.col] != '"');

			// To go over the last quote
			point.moveNext(true);

			token.end = point;
			buffer->tokens.push_back(token);
		} break;

		case '/':
		{
			Point nextPoint = point + 1;
			
			if (nextPoint.isInBuffer())
			{
				if (buffer->data[nextPoint.line][nextPoint.col] == '/')
				{
					Token token { Token::Type::LineComment, point };
					
					do
					{
						point.moveNext(true);
					} while (point.isInBuffer() && point.col < buffer->data[point.line].size());

					token.end = point;
					buffer->tokens.push_back(token);
				}
				else if (buffer->data[nextPoint.line][nextPoint.col] == '*')
				{
					Token token { Token::Type::BlockComment, point };
					point.moveNext(true);

					do
					{
						point.moveNext(true);

						if (point.isInBuffer() && buffer->data[point.line][point.col] == '*')
						{
							nextPoint = point + 1;

							if (nextPoint.isInBuffer() && buffer->data[nextPoint.line][nextPoint.col] == '/')
							{
								// To go over the ending slash
								point.moveNext(true);
								point.moveNext(true);
								
								break;
							}
						}
					} while (point.isInBuffer());

					token.end = point;
					buffer->tokens.push_back(token);
				}
				else
				{
					goto DEFAULT_CASE;
				}
			}
			else
			{
				goto DEFAULT_CASE;
			}
		} break;

		default:
		{
		DEFAULT_CASE:
			if (character >= '0' && character <= '9')
			{
				Token token { Token::Type::Number, point };

				do
				{
					point.moveNext();
					character = buffer->data[point.line][point.col];
				} while (character >= '0' && character <= '9');

				if (character == '.')
				{
					do
					{
						point.moveNext();
						character = buffer->data[point.line][point.col];
					} while (character >= '0' && character <= '9');
				}

				token.end = point;
				buffer->tokens.push_back(token);
			}
			else if (isIdentifierStartCharacter(character))
			{
				Point startPoint = point;
				std::string tokenText = "";

				do
				{
					tokenText += character;
					point.moveNext();
					character = buffer->data[point.line][point.col];
				} while (isIdentifierCharacter(character));

				if (std::find(Token::keywords.begin(), Token::keywords.end(), tokenText) != Token::keywords.end())
				{
					buffer->tokens.push_back({ Token::Type::Keyword, startPoint, point });
				}
				else
				{
					// TODO(fkp): Handle regular identifier
				}
			}
			else if (character == '#')
			{
				Token directiveToken { Token::Type::PreprocessorDirective, point };

				do
				{
					point.moveNext();
					character = buffer->data[point.line][point.col];
				} while (isIdentifierCharacter(character));

				directiveToken.end = point;
				buffer->tokens.push_back(directiveToken);

				if (point.col != buffer->data[point.line].size())
				{
					std::string tokenText = "";
					Point startPoint = point;
					
					// Skips space
					point.moveNext();
					character = buffer->data[point.line][point.col];

					do
					{
						tokenText += character;
						point.moveNext();
						character = buffer->data[point.line][point.col];
					} while (isIdentifierCharacter(character));

					if (tokenText == "defined")
					{
						buffer->tokens.push_back({ Token::Type::PreprocessorDirective, startPoint, point });
					}
				}
			}
			else
			{
				point.moveNext(true);
			}
		} break;
		}
	}
}

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
	case Token::Type::LineComment:				return normaliseColour(154, 154, 154, 255);
	case Token::Type::BlockComment:				return normaliseColour(154, 154, 154, 255);
	case Token::Type::Keyword:					return normaliseColour(184,   8, 180, 255);
	case Token::Type::PreprocessorDirective:	return normaliseColour(184,   8, 180, 255);

	default:									return getDefaultTextColour();
	}
}
