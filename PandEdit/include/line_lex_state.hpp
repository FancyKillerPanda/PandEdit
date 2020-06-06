//  ===== Date Created: 06 June, 2020 ===== 

#if !defined(LINE_LEX_STATE_HPP)
#define LINE_LEX_STATE_HPP

#include <vector>
#include "token.hpp"

class LineLexState
{
public:
	enum class FinishType
	{
		Finished,
		UnendedString,
		UnendedComment,
	};
	
	std::vector<Token> tokens;
	FinishType finishType = FinishType::Finished;

public:
	Token& getTokenBefore(int index, bool excludeComments = false, bool excludeAsteriskAndAmpersand = false);
	Token& getTokenBefore(int index, int& numberOfTokensTravelled, bool excludeComments = false, bool excludeAsteriskAndAmpersand = false);
	Token& getTokenAtOrAfter(int index, bool excludeComments = false, bool excludeAsteriskAndAmpersand = false);
	Token& getTokenAtOrAfter(int index, int& numberOfTokensTravelled, bool excludeComments = false, bool excludeAsteriskAndAmpersand = false);
};

#endif
