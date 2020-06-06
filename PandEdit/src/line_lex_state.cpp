//  ===== Date Created: 06 June, 2020 ===== 

#include "line_lex_state.hpp"

Token& LineLexState::getTokenBefore(int index, bool excludeComments, bool excludeAsteriskAndAmpersand)
{
	int unused = 0;
	return getTokenBefore(index, unused, excludeComments, excludeAsteriskAndAmpersand);
}

Token& LineLexState::getTokenBefore(int index, int& numberOfTokensTravelled, bool excludeComments, bool excludeAsteriskAndAmpersand)
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
		bool isAsteriskOrAmpersand = token.type == Token::Type::Asterisk || token.type == Token::Type::BitAnd;

		if (!((excludeComments && isComment) ||
			  (excludeAsteriskAndAmpersand && isAsteriskOrAmpersand)))
		{
			return token;
		}
	} while (true);
}

Token& LineLexState::getTokenAtOrAfter(int index, bool excludeComments, bool excludeAsteriskAndAmpersand)
{
	int unused = 0;
	return getTokenAtOrAfter(index, unused, excludeComments, excludeAsteriskAndAmpersand);
}

Token& LineLexState::getTokenAtOrAfter(int index, int& numberOfTokensTravelled, bool excludeComments, bool excludeAsteriskAndAmpersand)
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
		bool isAsteriskOrAmpersand = token.type == Token::Type::Asterisk || token.type == Token::Type::BitAnd;

		if (!((excludeComments && isComment) ||
			  (excludeAsteriskAndAmpersand && isAsteriskOrAmpersand)))
		{
			return token;
		}
		
		index += 1;
		numberOfTokensTravelled += 1;
	} while (true);
}
