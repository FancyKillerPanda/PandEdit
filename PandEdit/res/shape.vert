//  ===== Date Created: 14 April, 2020 ===== 

#version 330 core

layout (location = 0) in vec4 coord;
uniform mat4 projection;

void main()
{
	gl_Position = projection * coord;
}
