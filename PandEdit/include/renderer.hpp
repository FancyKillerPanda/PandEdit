//  ===== Date Created: 14 April, 2020 ===== 

#if !defined(RENDERER_HPP)
#define RENDERER_HPP

#include <string>
#include <utility>
#include <glad/glad.h>

#include "shader.hpp"

// TODO(fkp): Make this editable
constexpr unsigned int tabWidth = 4;

class Font;
class Matrix4;
class Frame;

struct TextToDraw
{
public:
	const std::string& text = "";
	int textLength = -1;

	float startX = -1.0f;
	float x = 0.0f;
	float y = 0.0f;
	
	float maxWidth = 0.0f;
	bool wrap = false;

public:
	TextToDraw(const std::string& text)
		: text(text)
	{
	}
};

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
	Renderer(const Matrix4& projection, float windowWidth, float windowHeight);

	void updateShaderUniforms(const Matrix4& projection, float windowWidth, float windowHeight);
	
	void drawRect(float x, float y, float width, float height);
	void drawHollowRect(float x, float y, float width, float height, float borderWidth);
//	std::pair<int, int> drawText(const std::string& text, int messageLength, float x, float y, float maxWidth = 0.0f, bool wrap = false, float startX = -1.0f);
	std::pair<int, int> drawText(TextToDraw& textToDraw);
	void drawFrame(Frame& frame);
};

#endif
