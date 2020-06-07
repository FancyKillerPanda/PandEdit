//  ===== Date Created: 06 June, 2020 ===== 

#if !defined(LINE_LEX_STATE_HPP)
#define LINE_LEX_STATE_HPP

#include <vector>
#include "token.hpp"

enum ExcludableToken
{
	EXCLUDE_NONE				= 0x00,
	EXCLUDE_COMMENT				= 0x01,
	EXCLUDE_ASTERISK			= 0x02,
	EXCLUDE_AMPERSAND			= 0x04,
	EXCLUDE_SCOPE_RESOLUTION	= 0x08,
	EXCLUDE_TYPE_BEFORE_SCOPE	= 0x10,
};

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
	// NOTE(fkp): Use ExcludableToken for the excludes
	Token& getTokenBefore(int index, int excludes);
	Token& getTokenBefore(int index, int& numberOfTokensTravelled, int excludes);
	Token& getTokenAtOrAfter(int index, int excludes);
	Token& getTokenAtOrAfter(int index, int& numberOfTokensTravelled, int excludes);
};

#endif
