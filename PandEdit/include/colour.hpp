//  ===== Date Created: 06 June, 2020 ===== 

#if !defined(COLOUR_HPP)
#define COLOUR_HPP

#include "token.hpp"

struct Colour
{
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 0.0f;
};


Colour normaliseColour(float r, float g, float b, float a);
Colour getDefaultTextColour();
Colour getColourForTokenType(Token::Type type); // TODO(fkp): This should have styling

#endif
