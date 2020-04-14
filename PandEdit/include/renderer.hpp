//  ===== Date Created: 14 April, 2020 ===== 

#if !defined(RENDERER_HPP)
#define RENDERER_HPP

#include <string>
#include <glad/glad.h>

#include "shader.hpp"

class Font;
class Matrix4;

class Renderer
{
	struct Vertex
	{
		GLfloat x;
		GLfloat y;
		GLfloat s;
		GLfloat t;
	};
	
public:
	Shader shapeShader;
	Shader textureShader;

	Font* currentFont;
	
public:
	Renderer(const Matrix4& projection);

	void updateProjection(const Matrix4& projection);
	
	void drawRect(float x, float y, float width, float height);
	void drawText(const std::string& text, unsigned int messageLength, float x, float y, float wrapWidth);
};

#endif
