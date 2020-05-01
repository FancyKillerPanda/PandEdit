//  ===== Date Created: 01 May, 2020 ===== 

#include "undo.hpp"

Action Action::insertion(Point start, Point end)
{
	Action result;
	
	result.type = ActionType::Insertion;
	result.start = start;
	result.end = end;

	return result;
}

Action Action::deletion(Point start, Point end, std::string data)
{
	Action result;
	
	result.type = ActionType::Deletion;
	result.start = start;
	result.end = end;
	result.data = data;

	return result;
}
