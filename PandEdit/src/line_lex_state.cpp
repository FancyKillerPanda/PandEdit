//  ===== Date Created: 06 June, 2020 ===== 

#include "line_lex_state.hpp"

bool isValidToken(Token& token, int excludes, Token::Type nextType = Token::Type::Invalid)
{
		bool isComment = token.type == Token::Type::LineComment || token.type == Token::Type::BlockComment;
		bool isAsterisk = token.type == Token::Type::Asterisk;
		bool isAmpersand = token.type == Token::Type::BitAnd;
		bool isScopeResolution = token.type == Token::Type::ScopeResolution;
		bool isType = token.type == Token::Type::TypeName;

		bool isInvalid = ((excludes & EXCLUDE_COMMENT) && isComment) ||
						 ((excludes & EXCLUDE_ASTERISK) && isAsterisk) ||
						 ((excludes & EXCLUDE_AMPERSAND) && isAmpersand) ||
						 ((excludes & EXCLUDE_SCOPE_RESOLUTION) && isScopeResolution) ||
						 ((excludes & EXCLUDE_TYPE_BEFORE_SCOPE) && isType && nextType == Token::Type::ScopeResolution);
		
		return !isInvalid;
}

Token* LineLexState::getTokenBefore(int index, int excludes)
{
	int unused = 0;
	return getTokenBefore(index, unused, excludes);
}

Token* LineLexState::getTokenBefore(int index, int& numberOfTokensTravelled, int excludes)
{
	numberOfTokensTravelled = 0;
	Token::Type previousTokenType = Token::Type::Invalid;

	do
	{
		index -= 1;
		
		if (index < 0 || index >= tokens.size())
		{
			return nullptr;
		}

		numberOfTokensTravelled += 1;
		Token& token = tokens[index];
		
		if (isValidToken(token, excludes, previousTokenType))
		{
			return &token;
		}
		
		previousTokenType = token.type;
	} while (true);
}

Token* LineLexState::getTokenAtOrAfter(int index, int excludes)
{
	int unused = 0;
	return getTokenAtOrAfter(index, unused, excludes);
}

Token* LineLexState::getTokenAtOrAfter(int index, int& numberOfTokensTravelled, int excludes)
{
	numberOfTokensTravelled = 0;
	
	do
	{
		if (index < 0 || index >= tokens.size())
		{
			return nullptr;
		}

		Token& token = tokens[index];
		Token::Type nextTokenType = Token::Type::Invalid;

		if (index > 0)
		{
			nextTokenType = tokens[index - 1].type;
		}
		
		
		if (!isValidToken(token, excludes, nextTokenType))
		{
			return &token;
		}
		
		index += 1;
		numberOfTokensTravelled += 1;
	} while (true);
}
