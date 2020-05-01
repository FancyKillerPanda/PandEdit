//  ===== Date Created: 01 May, 2020 ===== 

#if !defined(UNDO_HPP)
#define UNDO_HPP

#include <string>
#include "point.hpp"

enum class ActionType
{
	Insertion,
	Deletion,
};

// TODO(fkp): I don't like this name
class Action
{
public:
	ActionType type;
	std::string data = "";
	
	Point start;
	Point end;

public:
	static Action insertion(Point start, Point end, std::string data);
	static Action deletion(Point start, Point end, std::string data);
};

#endif
