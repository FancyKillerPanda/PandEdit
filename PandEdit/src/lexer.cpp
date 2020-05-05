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

		default:
		{
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
