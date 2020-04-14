//  ===== Date Created: 14 April, 2020 ===== 

#version 330 core

in vec2 texCoords;
out vec4 colour;

uniform sampler2D text;
uniform vec4 textColour;

void main()
{
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, texCoords).r);
	colour = textColour * sampled;
}
