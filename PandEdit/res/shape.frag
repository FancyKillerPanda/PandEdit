//  ===== Date Created: 14 April, 2020 ===== 

#version 330 core

out vec4 outColour;

uniform vec2 resolution;
uniform vec4 rectangleDimensions;
uniform float borderWidth;
uniform vec4 colour;

void main()
{
	if (borderWidth == 0.0)
	{
		outColour = colour;
	}
	else
	{
		float visible = 0.0;

		// In the order: left, bottom, right, top
		if ((gl_FragCoord.x - rectangleDimensions.x < borderWidth) ||
			(gl_FragCoord.y - (resolution.y - (rectangleDimensions.y + rectangleDimensions.w)) < borderWidth) ||
			((rectangleDimensions.x + rectangleDimensions.z) - gl_FragCoord.x < borderWidth) ||
			((resolution.y - rectangleDimensions.y) - gl_FragCoord.y < borderWidth))
		{
			visible = 1.0;
		}

		outColour = colour * vec4(visible);
	}
}
