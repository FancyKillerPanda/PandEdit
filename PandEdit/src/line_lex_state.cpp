//  ===== Date Created: 06 June, 2020 ===== 

#include "line_lex_state.hpp"

Token& LineLexState::getTokenBefore(int index, int excludes)
{
	int unused = 0;
	return getTokenBefore(index, unused, excludes);
}

Token& LineLexState::getTokenBefore(int index, int& numberOfTokensTravelled, int excludes)
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
		bool isComment = token.type == Token::Type::LineComment || token.type == Token::Type::BlockComment;
		bool isAsterisk = token.type == Token::Type::Asterisk;
		bool isAmpersand = token.type == Token::Type::BitAnd;
		bool isScopeResolution = token.type == Token::Type::ScopeResolution;

		bool isInvalid = ((excludes & EXCLUDE_COMMENT) && isComment) ||
						 ((excludes & EXCLUDE_ASTERISK) && isAsterisk) ||
						 ((excludes & EXCLUDE_AMPERSAND) && isAmpersand) ||
						 ((excludes & EXCLUDE_SCOPE_RESOLUTION) && isScopeResolution);
		
		if (!isInvalid)
		{
			return token;
		}
	} while (true);
}

Token& LineLexState::getTokenAtOrAfter(int index, int excludes)
{
	int unused = 0;
	return getTokenAtOrAfter(index, unused, excludes);
}

Token& LineLexState::getTokenAtOrAfter(int index, int& numberOfTokensTravelled, int excludes)
{
	numberOfTokensTravelled = 0;
	
	do
	{
		if (index < 0 || index >= tokens.size())
		{
			return Token { Token::Type::Invalid, { 0, 0 }, { 0, 0 }, "" };
		}

		Token& token = tokens[index];
		bool isComment = token.type == Token::Type::LineComment || token.type == Token::Type::BlockComment;
		bool isAsterisk = token.type == Token::Type::Asterisk;
		bool isAmpersand = token.type == Token::Type::BitAnd;
		bool isScopeResolution = token.type == Token::Type::ScopeResolution;

		bool isInvalid = ((excludes & EXCLUDE_COMMENT) && isComment) ||
						 ((excludes & EXCLUDE_ASTERISK) && isAsterisk) ||
						 ((excludes & EXCLUDE_AMPERSAND) && isAmpersand) ||
						 ((excludes & EXCLUDE_SCOPE_RESOLUTION) && isScopeResolution);
		
		if (!isInvalid)
		{
			return token;
		}
		
		index += 1;
		numberOfTokensTravelled += 1;
	} while (true);
}
