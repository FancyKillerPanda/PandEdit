//  ===== Date Created: 04 May, 2020 ===== 

#include <algorithm>

#include "lexer.hpp"
#include "buffer.hpp"
#include "common.hpp"

// 1. NOTE(fkp): These are only keywords in some contexts
std::unordered_set<std::string> Lexer::keywords = {
	"alignas", "alignof", "sizeof", "typeid", "decltype",
	
	"and", "and_eq", "bitand", "bitor", "compl",
	"not", "not_eq", "or", "or_eq", "xor", "xor_eq",
	
	"atomic_cancel", "atomic_commit", "atomic_noexcept",

	"break", "case", "continue", "default", "do", "else",
	"for", "goto", "if", "return", "switch", "while",

	"const", "consteval", "constexpr", "constinit", "const_cast",
	
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

std::unordered_set<std::string> Lexer::primitiveTypes = {
	"bool",
	"char", "char8_t", "char16_t", "char32_t", "wchar_t",
	"double", "float",
	"int", "long", "short",
	"signed", "unsigned",
	"false", "nullptr", "true", "void",
};

Lexer::Lexer(Buffer* buffer)
	: buffer(buffer)
{
}

#define UPDATE_CHARACTER() character = buffer->data[point.line][point.col]
#define LINE_STATE lineStates[point.line]
#define LINE_TOKENS lineStates[point.line].tokens

void Lexer::lex(unsigned int startLine, bool lexEntireBuffer)
{
	if (buffer->data.size() == 0 ||
		(buffer->data.size() == 1 && buffer->data[0].size() == 0))
	{
		lineStates.clear();
		return;
	}

	if (lineStates.size() > buffer->data.size())
	{
		ERROR_ONCE("Error: More line states than buffer lines.\n");
		return;
	}
	
	while (lineStates.size() < buffer->data.size())
	{
		// TODO(fkp): Maybe figure this out better
		lineStates.emplace_back();
	}
	
	Point point { startLine, 0, buffer };
	LineLexState::FinishType currentLineLastFinishType = lineStates[point.line].finishType;

	while (point.isInBuffer())
	{
		if (point.col == 0)
		{
			LINE_TOKENS.clear();

			if (point.line > 0)
			{
				if (lineStates[point.line - 1].finishType == LineLexState::FinishType::UnendedString)
				{
					lexString(point, currentLineLastFinishType);
				}
				else if (lineStates[point.line - 1].finishType == LineLexState::FinishType::UnendedComment)
				{
					lexBlockComment(point, currentLineLastFinishType);
				}
			}
		}

		if (!point.isInBuffer())
		{
			break;
		}
		
		char character;
		UPDATE_CHARACTER();

		switch (character)
		{
		case '"':
		{
			lexString(point, currentLineLastFinishType);
		} break;

		case '\'':
		{
			lexCharacter(point);
		} break;

		case '<':
		{
			const Token& lastToken = LINE_STATE.getTokenBefore(LINE_TOKENS.size(), true);
			
			if (lastToken.type != Token::Type::PreprocessorDirective ||
				lastToken.data != "include")
			{
				goto DEFAULT_CASE;
			}
			
			lexIncludePath(point);
		} break;

		case '/':
		{
			Point nextPoint = point + 1;

			if (nextPoint.isInBuffer())
			{
				if (buffer->data[nextPoint.line][nextPoint.col] == '/')
				{
					lexLineComment(point);
				}
				else if (buffer->data[nextPoint.line][nextPoint.col] == '*')
				{
					lexBlockComment(point, currentLineLastFinishType);
				}
				else
				{
					goto DEFAULT_CASE;
				}
			}
		} break;

		case '#':
		{
			lexPreprocessorDirective(point);
		} break;

		default:
		{
		DEFAULT_CASE:
			if (point.col == buffer->data[point.line].size())
			{
				lineStates[point.line].finishType = LineLexState::FinishType::Finished;

				if (lineStates[point.line].finishType == currentLineLastFinishType &&
					!lexEntireBuffer)
				{
					goto FINISHED_LEX;
				}
				else
				{
					point.moveNext(true);
					
					if (point.isInBuffer())
					{
						LINE_TOKENS.clear();
					}
				}
			}
			else if (character >= '0' && character <= '9')
			{
				lexNumber(point);
			}
			else if (isIdentifierStartCharacter(character))
			{
				Point startPoint = point;
				std::string tokenText;
				char character;
				UPDATE_CHARACTER();

				do
				{
					tokenText += character;
					point.moveNext();
					UPDATE_CHARACTER();
				} while (isIdentifierCharacter(character));
				
				if (!lexKeyword(startPoint, point, tokenText))
				{
					lexIdentifier(startPoint, point, tokenText);
				}
			}
			else
			{
			OTHER_CHARACTER:
				if (!lexPunctuation(point))
				{
					point.moveNext(true);
				}
			}
		} break;
		}
	}

FINISHED_LEX:
	doFinalAdjustments();
}

void Lexer::addLine(Point splitPoint)
{
	lineStates.emplace(lineStates.begin() + splitPoint.line + 1);

	for (int i = 0; i < lineStates[splitPoint.line].tokens.size(); i++)
	{
		Token& token = lineStates[splitPoint.line].tokens[i];

		if (token.start.col >= splitPoint.col)
		{
			std::move(lineStates[splitPoint.line].tokens.begin() + i, lineStates[splitPoint.line].tokens.end(), std::back_inserter(lineStates[splitPoint.line + 1].tokens));

			lineStates[splitPoint.line + 1].finishType = lineStates[splitPoint.line].finishType;			
			lineStates[splitPoint.line].tokens.erase(lineStates[splitPoint.line].tokens.begin() + i, lineStates[splitPoint.line].tokens.end());

			for (Token& token : lineStates[splitPoint.line + 1].tokens)
			{
				token.start.col -= splitPoint.col;
				token.end.col -= splitPoint.col;
			}
			
			break;
		}
	}

	for (int i = splitPoint.line + 1; i < buffer->data.size(); i++)
	{
		for (Token& token : lineStates[i].tokens)
		{
			token.start.line += 1;
			token.end.line += 1;
		}
	}
}

void Lexer::removeLine(Point newPoint)
{
	for (Token& token : lineStates[newPoint.line + 1].tokens)
	{
		token.start.line -= 1;
		token.start.col += newPoint.col;
		token.end.line -= 1;
		token.end.col += newPoint.col;
	}

	std::move(lineStates[newPoint.line + 1].tokens.begin(), lineStates[newPoint.line + 1].tokens.end(), std::back_inserter(lineStates[newPoint.line].tokens));
	lineStates[newPoint.line].finishType = lineStates[newPoint.line + 1].finishType;
	lineStates.erase(lineStates.begin() + newPoint.line + 1);

	for (int i = newPoint.line + 1; i < buffer->data.size(); i++)
	{
		for (Token& token : lineStates[i].tokens)
		{
			token.start.line -= 1;
			token.end.line -= 1;
		}
	}
}

std::vector<Token> Lexer::getTokens(unsigned int startLine, unsigned int endLine)
{
	if (lineStates.size() == 0)
	{
		return {};
	}

	if (lineStates.size() < endLine)
	{
		ERROR_ONCE("Error: getTokens() ending line is greater than number of lines.\n");
		return {};
	}

	std::vector<Token> result;

	// TODO(fkp): Merging
	for (int i = startLine; i <= endLine; i++)
	{
		for (Token token : lineStates[i].tokens)
		{
			result.push_back(token);
		}
	}

	return result;
}

void Lexer::lexString(Point& point, LineLexState::FinishType& currentLineLastFinishType)
{
	char character;
	Point startPoint = point;
			
	do
	{
		point.moveNext(true);

		if (!point.isInBuffer())
		{
			break;
		}

		// This happens when we go to the next line after an unended string
		if (point.col == 0)
		{
			startPoint = point;
			LINE_TOKENS.clear();
			currentLineLastFinishType = lineStates[point.line].finishType;
		}
				
		UPDATE_CHARACTER();

		if (character == '"')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::String, startPoint, point);
			lineStates[point.line].finishType = LineLexState::FinishType::Finished;

			break;
		}
		else if (point.col == buffer->data[point.line].size())
		{
			LINE_TOKENS.emplace_back(Token::Type::String, startPoint, point);
			lineStates[point.line].finishType = LineLexState::FinishType::UnendedString;
		}
		else if (character == '\\')
		{
			// This is safe because we know it's not at the end of the line
			char nextCharacter = buffer->data[point.line][point.col + 1];

			// TODO(fkp): Unicode/hex
			const std::string escapeCharacters = "'\"?\\abfnrtv";

			// TODO(fkp): This whole block is quite repetitive
			if (escapeCharacters.find(nextCharacter) != std::string::npos)
			{
				LINE_TOKENS.emplace_back(Token::Type::String, startPoint, point);
				lineStates[point.line].finishType = LineLexState::FinishType::UnendedString;

				startPoint = point;
				point.moveNext();
				LINE_TOKENS.emplace_back(Token::Type::EscapeSequence, startPoint, point + 1);

				startPoint = point + 1;
			}
			else if (nextCharacter >= '0' && nextCharacter <= '7')
			{
				LINE_TOKENS.emplace_back(Token::Type::String, startPoint, point);
				lineStates[point.line].finishType = LineLexState::FinishType::UnendedString;

				startPoint = point;
				point.moveNext();
				nextCharacter = buffer->data[point.line][point.col + 1];

				if (nextCharacter >= '0' && nextCharacter <= '7')
				{
					point.moveNext();
					nextCharacter = buffer->data[point.line][point.col + 1];

					if (nextCharacter >= '0' && nextCharacter <= '7')
					{
						point.moveNext();
					}
				}

				LINE_TOKENS.emplace_back(Token::Type::EscapeSequence, startPoint, point + 1);
				startPoint = point + 1;
			}
			else if (nextCharacter == 'x')
			{
				nextCharacter = buffer->data[point.line][point.col + 2];

				if (isValidHexDigit(nextCharacter))
				{
					LINE_TOKENS.emplace_back(Token::Type::String, startPoint, point);
					lineStates[point.line].finishType = LineLexState::FinishType::UnendedString;
					startPoint = point;
					point.moveNext();
					point.moveNext();
					UPDATE_CHARACTER();

					while (isValidHexDigit(character))
					{
						point.moveNext();
						UPDATE_CHARACTER();
					}
					
					LINE_TOKENS.emplace_back(Token::Type::EscapeSequence, startPoint, point);
					startPoint = point;
					
					// This has to be done because the next iteration
					// of the loop will call moveNext()
					point.movePrevious();
				}
			}
			else if (nextCharacter == 'u' || nextCharacter == 'U')
			{
				Point stringStartPoint = startPoint;
				Point escapeStartPoint = point;
				int numberOfDigitsNeeded = 4;
				int numberOfDigits = 0;

				if (nextCharacter == 'U')
				{
					numberOfDigitsNeeded = 8;
				}

				nextCharacter = buffer->data[point.line][point.col + 2];
				
				if (isValidHexDigit(nextCharacter))
				{
					point.moveNext();
					point.moveNext();
					UPDATE_CHARACTER();

					while (isValidHexDigit(character))
					{
						point.moveNext();
						UPDATE_CHARACTER();
						
						if (numberOfDigits++ >= numberOfDigitsNeeded)
						{
							break;
						}
					}
				}

				if (numberOfDigits == numberOfDigitsNeeded)
				{
					LINE_TOKENS.emplace_back(Token::Type::String, stringStartPoint, escapeStartPoint);
					lineStates[point.line].finishType = LineLexState::FinishType::UnendedString;

					LINE_TOKENS.emplace_back(Token::Type::EscapeSequence, escapeStartPoint, point);
					startPoint = point;
					point.movePrevious();
				}
				else if (point.col == buffer->data[point.line].size())
				{
					LINE_TOKENS.emplace_back(Token::Type::String, startPoint, point);
					lineStates[point.line].finishType = LineLexState::FinishType::UnendedString;
				}
			}
		}
	} while (true);
}

void Lexer::lexCharacter(Point& point)
{
	char character;
	Point startPoint = point;
	point.moveNext(true);

	if (point.isInBuffer())
	{
		UPDATE_CHARACTER();

		if (character == '\\')
		{
			point.moveNext(true);
		}

		point.moveNext(true);
				
		if (point.isInBuffer())
		{
			UPDATE_CHARACTER();

			if (character == '\'')
			{
				point.moveNext(true);
				LINE_TOKENS.emplace_back(Token::Type::Character, startPoint, point);
			}
		}
	}
}

void Lexer::lexIncludePath(Point& point)
{
	char character;
	Point startPoint = point;
	point.moveNext(true);
			
	while (point.isInBuffer())
	{
		UPDATE_CHARACTER();
				
		if (character == '>')
		{
			point.moveNext(true);
			break;
		}
		else if (point.col == buffer->data[point.line].size())
		{
			break;
		}
		else
		{
			point.moveNext(true);
		}
	}

	LINE_TOKENS.emplace_back(Token::Type::IncludeAngleBracketPath, startPoint, point);
}

void Lexer::lexLineComment(Point& point)
{
	Point startPoint = point;

	do
	{
		point.moveNext(true);
	} while (point.isInBuffer() && point.col < buffer->data[point.line].size());

	LINE_TOKENS.emplace_back(Token::Type::LineComment, startPoint, point);
}

void Lexer::lexBlockComment(Point& point, LineLexState::FinishType& currentLineLastFinishType)
{
	char character;
	Point startPoint = point;

	do
	{
		point.moveNext(true);

		if (!point.isInBuffer())
		{
			break;
		}

		// This happens when we go to the next line after an unended comment
		if (point.col == 0)
		{
			startPoint = point;
			LINE_TOKENS.clear();
			currentLineLastFinishType = lineStates[point.line].finishType;
		}

		UPDATE_CHARACTER();

		if (character == '*' &&
			point.col < buffer->data[point.line].size() - 1 &&
			buffer->data[point.line][point.col + 1] == '/')
		{
			point.moveNext();
			point.moveNext();

			LINE_TOKENS.emplace_back(Token::Type::BlockComment, startPoint, point);
			lineStates[point.line].finishType = LineLexState::FinishType::Finished;

			break;
		}
		else if (point.col == buffer->data[point.line].size())
		{
			LINE_TOKENS.emplace_back(Token::Type::BlockComment, startPoint, point);
			lineStates[point.line].finishType = LineLexState::FinishType::UnendedComment;
		}
	} while (true);
}

void Lexer::lexNumber(Point& point)
{
	char character;
	Token token { Token::Type::Number, point };

	do
	{
		point.moveNext();
		UPDATE_CHARACTER();
	} while (isValidHexDigit(character) || character == '\'');

	if (character == '.' ||
		character == 'x' || character == 'X' ||
		character == 'b' || character == 'B')
	{
		do
		{
			point.moveNext();
			UPDATE_CHARACTER();
		} while (isValidHexDigit(character) || character == '\'');
	}

	// Suffixes
	while (character == 'u' || character == 'U' ||
		   character == 'l' || character == 'L')
	{
		point.moveNext();
		UPDATE_CHARACTER();
	}
				
	token.end = point;
	LINE_TOKENS.push_back(token);
}

void Lexer::lexPreprocessorDirective(Point& point)
{
	char character;
	UPDATE_CHARACTER();
	Point startPoint = point;
	std::string tokenText = "";

	do
	{
		if (character != '#' && character != ' ')
		{
			tokenText += character;
		}
		
		point.moveNext();
		UPDATE_CHARACTER();

		if (isspace(character) && tokenText != "")
		{
			break;
		}
	} while (isIdentifierCharacter(character) || isspace(character));

	tokenText.erase(std::remove_if(tokenText.begin(), tokenText.end(), isspace));
	LINE_TOKENS.emplace_back(Token::Type::PreprocessorDirective, startPoint, point, tokenText);
}

bool Lexer::lexKeyword(const Point& startPoint, const Point& point, const std::string& tokenText)
{
	if (keywords.find(tokenText) != keywords.end())
	{
		LINE_TOKENS.emplace_back(Token::Type::Keyword, startPoint, point, tokenText);
	}
	else if (tokenText == "defined")
	{
		bool foundIfElifDirectiveOnLine = false;

		for (const Token& token : LINE_TOKENS)
		{
			if (token.type == Token::Type::PreprocessorDirective &&
				(token.data == "if" || token.data == "elif"))
			{
				foundIfElifDirectiveOnLine = true;
			}
		}

		if (foundIfElifDirectiveOnLine)
		{
			LINE_TOKENS.emplace_back(Token::Type::PreprocessorDirective, startPoint, point, tokenText);
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

void Lexer::lexIdentifier(const Point& startPoint, const Point& point, const std::string& tokenText)
{
	const Token& lastToken = LINE_STATE.getTokenBefore(LINE_TOKENS.size(), true);

	if (lastToken.type == Token::Type::PreprocessorDirective &&
		lastToken.data == "define")
	{
		LINE_TOKENS.emplace_back(Token::Type::MacroName, startPoint, point);
	}
	else if (primitiveTypes.find(tokenText) != primitiveTypes.end())
	{
		LINE_TOKENS.emplace_back(Token::Type::TypeName, startPoint, point);
	}
	else
	{
		LINE_TOKENS.emplace_back(Token::Type::IdentifierUsage, startPoint, point);
	}
}

bool Lexer::lexPunctuation(Point& point)
{
	char character;
	UPDATE_CHARACTER();

	Point startPoint = point;
	point.moveNext();

	switch (character)
	{
	case '(':
	{
		LINE_TOKENS.emplace_back(Token::Type::LeftParen, startPoint, point);
	} break;
	
	case ')':
	{
		LINE_TOKENS.emplace_back(Token::Type::RightParen, startPoint, point);
	} break;
	
	case '{':
	{
		LINE_TOKENS.emplace_back(Token::Type::LeftBrace, startPoint, point);
	} break;
	
	case '}':
	{
		LINE_TOKENS.emplace_back(Token::Type::RightBrace, startPoint, point);
	} break;
	
	case '[':
	{
		LINE_TOKENS.emplace_back(Token::Type::LeftBracket, startPoint, point);
	} break;
	
	case ']':
	{
		LINE_TOKENS.emplace_back(Token::Type::RightBracket, startPoint, point);
	} break;
	
	case '<':
	{
		UPDATE_CHARACTER();

		if (character == '=')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::LessEqual, startPoint, point);			
		}
		else if (character == '<')
		{
			point.moveNext();
			UPDATE_CHARACTER();

			if (character == '=')
			{
				point.moveNext();
				UPDATE_CHARACTER();

				if (character == '>')
				{
					point.moveNext();
					LINE_TOKENS.emplace_back(Token::Type::Spaceship, startPoint, point);
					
				}
				else
				{
					LINE_TOKENS.emplace_back(Token::Type::ShiftLeftEqual, startPoint, point);
				}
			}
			else
			{
				LINE_TOKENS.emplace_back(Token::Type::ShiftLeft, startPoint, point);
			}
		}
		else
		{
			LINE_TOKENS.emplace_back(Token::Type::Less, startPoint, point);
		}
	} break;
	
	case '>':
	{
		UPDATE_CHARACTER();

		if (character == '=')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::GreaterEqual, startPoint, point);
		}
		else if (character == '>')
		{
			point.moveNext();
			UPDATE_CHARACTER();

			if (character == '=')
			{
				point.moveNext();
				LINE_TOKENS.emplace_back(Token::Type::ShiftRightEqual, startPoint, point);
			}
			else
			{
				LINE_TOKENS.emplace_back(Token::Type::ShiftRight, startPoint, point);
			}
		}
		else
		{
			LINE_TOKENS.emplace_back(Token::Type::Greater, startPoint, point);
		}
	} break;
	
	case '=':
	{
		UPDATE_CHARACTER();

		if (character == '=')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::EqualEqual, startPoint, point);
		}
		else
		{
			LINE_TOKENS.emplace_back(Token::Type::Equal, startPoint, point);
		}
	} break;
	
	case '!':
	{
		UPDATE_CHARACTER();

		if (character == '=')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::BangEqual, startPoint, point);
		}
		else
		{
			LINE_TOKENS.emplace_back(Token::Type::Bang, startPoint, point);
		}
	} break;
	
	case '&':
	{
		UPDATE_CHARACTER();

		if (character == '=')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::BitAndEqual, startPoint, point);
		}
		else if (character == '&')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::LogicalAnd, startPoint, point);
		}
		else
		{
			LINE_TOKENS.emplace_back(Token::Type::BitAnd, startPoint, point);
		}
	} break;
		
	case '|':
	{
		UPDATE_CHARACTER();

		if (character == '=')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::BitOrEqual, startPoint, point);
		}
		else if (character == '|')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::LogicalOr, startPoint, point);
		}
		else
		{
			LINE_TOKENS.emplace_back(Token::Type::BitOr, startPoint, point);
		}
	} break;
	
	case '^':
	{
		UPDATE_CHARACTER();

		if (character == '=')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::BitXorEqual, startPoint, point);
		}
		else
		{
			LINE_TOKENS.emplace_back(Token::Type::BitXor, startPoint, point);
		}
	} break;
	
	case '~':
	{
		LINE_TOKENS.emplace_back(Token::Type::BitNot, startPoint, point);
	} break;
	
	case '+':
	{
		UPDATE_CHARACTER();

		if (character == '=')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::PlusEqual, startPoint, point);
		}
		else if (character == '+')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::Increment, startPoint, point);
		}
		else
		{
			LINE_TOKENS.emplace_back(Token::Type::Plus, startPoint, point);
		}
	} break;
	
	case '-':
	{
		UPDATE_CHARACTER();

		if (character == '=')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::MinusEqual, startPoint, point);
		}
		else if (character == '-')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::Decrement, startPoint, point);
		}
		else if (character == '>')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::Arrow, startPoint, point);			
		}
		else
		{
			LINE_TOKENS.emplace_back(Token::Type::Minus, startPoint, point);
		}
	} break;
	
	case '*':
	{
		UPDATE_CHARACTER();

		if (character == '=')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::AsteriskEqual, startPoint, point);
		}
		else
		{
			LINE_TOKENS.emplace_back(Token::Type::Asterisk, startPoint, point);
		}
	} break;
	
	case '/':
	{
		UPDATE_CHARACTER();

		if (character == '=')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::SlashEqual, startPoint, point);
		}
		else
		{
			LINE_TOKENS.emplace_back(Token::Type::Slash, startPoint, point);
		}
	} break;
	
	case '%':
	{
		UPDATE_CHARACTER();

		if (character == '=')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::PercentEqual, startPoint, point);
		}
		else
		{
			LINE_TOKENS.emplace_back(Token::Type::Percent, startPoint, point);
		}
	} break;
	
	case '.':
	{
		LINE_TOKENS.emplace_back(Token::Type::Dot, startPoint, point);
	} break;
	
	case ',':
	{
		LINE_TOKENS.emplace_back(Token::Type::Comma, startPoint, point);
	} break;
	
	case '?':
	{
		LINE_TOKENS.emplace_back(Token::Type::Question, startPoint, point);
	} break;
	
	case ':':
	{
		UPDATE_CHARACTER();

		if (character == ':')
		{
			point.moveNext();
			LINE_TOKENS.emplace_back(Token::Type::ScopeResolution, startPoint, point);
		}
		else
		{
			LINE_TOKENS.emplace_back(Token::Type::Colon, startPoint, point);
		}
	} break;

	case ';':
	{
		LINE_TOKENS.emplace_back(Token::Type::Semicolon, startPoint, point);
	} break;

	default:
	{
		// Undoes the moveNext() at the start of this method
		point.movePrevious();
	} return false;
	}

	return true;
}

void Lexer::doFinalAdjustments()
{
	// TODO(fkp): Use semicolons instead of lines
	for (LineLexState& lineState : lineStates)
	{
		for (int i = 0; i < lineState.tokens.size(); i++)
		{
			if (lineState.tokens[i].type == Token::Type::ScopeResolution)
			{
				Token& lastToken = lineState.getTokenBefore(i);
				
				if (lastToken.type == Token::Type::IdentifierUsage)
				{
					lastToken.type = Token::Type::TypeName;
				}
			}
			else if (lineState.tokens[i].type == Token::Type::LeftParen)
			{
				Token& lastToken = lineState.getTokenBefore(i, true);
				
				if (i == 1)
				{
					if (lastToken.type == Token::Type::IdentifierUsage)
					{
						lastToken.type = Token::Type::FunctionUsage;
					}
				}
				else if (i > 1)
				{
					if (lastToken.type == Token::Type::IdentifierUsage)
					{
						Token& tokenBeforeLast = lineState.getTokenBefore(i - 1, true);
						
						if (tokenBeforeLast.type == Token::Type::IdentifierUsage ||
							tokenBeforeLast.type == Token::Type::TypeName)
						{
							lastToken.type = Token::Type::FunctionDefinition;
							tokenBeforeLast.type = Token::Type::TypeName;
						}
						else
						{
							lastToken.type = Token::Type::FunctionUsage;
						}
					}
				}
			}
			else if (lineState.tokens[i].type == Token::Type::Equal ||
					 lineState.tokens[i].type == Token::Type::Semicolon ||
					 lineState.tokens[i].type == Token::Type::Comma ||
					 lineState.tokens[i].type == Token::Type::RightParen)
			{
				int numberOfTokensBack = 0;
				Token& lastToken = lineState.getTokenBefore(i, numberOfTokensBack, true);
				Token& tokenBeforeLast = lineState.getTokenBefore(i - numberOfTokensBack, true);
				
				if (lastToken.type == Token::Type::IdentifierUsage &&
					(tokenBeforeLast.type == Token::Type::IdentifierUsage ||
					 tokenBeforeLast.type == Token::Type::TypeName))
				{
					lastToken.type = Token::Type::IdentifierDefinition;
					tokenBeforeLast.type = Token::Type::TypeName;
				}
			}
		}
		
		// #error shouldn't have syntax highlighting
		// TODO(fkp): This should get the first non-comment token
		Token& firstToken = lineState.getTokenBefore(1);
		
		if (firstToken.type == Token::Type::PreprocessorDirective &&
			firstToken.data == "error")
		{
			for (int i = 1; i < lineState.tokens.size();)
			{
				if (lineState.tokens[i].type != Token::Type::LineComment &&
					lineState.tokens[i].type != Token::Type::BlockComment &&
					lineState.tokens[i].type != Token::Type::String)
				{
					lineState.tokens.erase(lineState.tokens.begin() + i);
				}
				else
				{
					i += 1;
				}
			}
		}
	}
}

bool Lexer::isIdentifierStartCharacter(char character)
{
	bool lowercaseAToZ = character >= 'a' && character <= 'z';
	bool uppercaseAToZ = character >= 'A' && character <= 'Z';
	bool underscore = character == '_';
	
	return lowercaseAToZ || uppercaseAToZ || underscore;
}

bool Lexer::isIdentifierCharacter(char character)
{
	bool number = character >= '0' && character <= '9';
	return isIdentifierStartCharacter(character) || number;
}

bool Lexer::isValidHexDigit(char character)
{
	bool number = character >= '0' && character <= '9';
	bool lowercaseHex = character >= 'a' && character <= 'f';
	bool uppercaseHex = character >= 'A' && character <= 'F';

	return number || lowercaseHex || uppercaseHex;
}

Token& LineLexState::getTokenBefore(int index, bool excludeComments)
{
	int unused = 0;
	return getTokenBefore(index, unused, excludeComments);
}

Token& LineLexState::getTokenBefore(int index, int& numberOfTokensTravelled, bool excludeComments)
{
	numberOfTokensTravelled = 0;
	
	do
	{
		index -= 1;
		
		if (index < 0 || index >= tokens.size())
		{
			return Token { Token::Type::Invalid, { 0, 0 }, { 0, 0 }, "" };
		}

		numberOfTokensTravelled += 1;
		Token& token = tokens[index];

		if (!(excludeComments &&
			  (token.type == Token::Type::LineComment ||
			   token.type == Token::Type::BlockComment)))
		{
			return token;
		}
	} while (true);
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
