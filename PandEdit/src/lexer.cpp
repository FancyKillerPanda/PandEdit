//  ===== Date Created: 04 May, 2020 ===== 

#include "lexer.hpp"
#include "buffer.hpp"

std::vector<Token> lexCppBuffer(const Buffer* buffer)
{
	// Empty buffer
	if (buffer->data.size() == 0 ||
		(buffer->data.size() == 1 && buffer->data[0].size() == 0))
	{
		return {};
	}

	Point point { buffer };
	std::vector<Token> result;

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
				point++;
			} while (buffer->data[point.line][point.col] != '"');

			// To go over the last quote
			point++;

			token.end = point;
			result.push_back(token);
		} break;
			
		case '"':
		{
			Token token { Token::Type::String, point };
			
			do
			{
				point++;
			} while (buffer->data[point.line][point.col] != '"');

			// To go over the last quote
			point++;

			token.end = point;
			result.push_back(token);
		} break;

		default:
		{
			if (character >= '0' && character <= '9')
			{
				Token token { Token::Type::Number, point };

				do
				{
					point++;
					character = buffer->data[point.line][point.col];
				} while (character >= '0' && character <= '9');

				if (character == '.')
				{
					do
					{
						point++;
						character = buffer->data[point.line][point.col];
					} while (character >= '0' && character <= '9');
				}

				token.end = point;
				result.push_back(token);
			}
			else
			{
				point++;
			}
		} break;
		}
	}

	return result;
}
