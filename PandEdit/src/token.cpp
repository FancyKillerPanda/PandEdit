//  ===== Date Created: 06 June, 2020 ===== 

#include "token.hpp"

Token::Token(Type type, Point start, std::string data)
	: type(type), start(start), data(data)
{
}

Token::Token(Type type, Point start, Point end, std::string data)
	: type(type), start(start), end(end), data(data)
{
}
