//  ===== Date Created: 04 May, 2020 ===== 

#include "lexer.hpp"
#include "buffer.hpp"

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
	case Token::Type::Number:			return normaliseColour(255, 174,   0, 255);
	case Token::Type::Character:		return normaliseColour(  0, 160,   9, 255);
	case Token::Type::String:			return normaliseColour(  0, 160,   9, 255);
	case Token::Type::LineComment:		return normaliseColour(154, 154, 154, 255);
	case Token::Type::BlockComment:		return normaliseColour(154, 154, 154, 255);

	default:						return getDefaultTextColour();
	}
}
