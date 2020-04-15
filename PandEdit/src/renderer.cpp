//  ===== Date Created: 14 April, 2020 =====

#include "renderer.hpp"
#include "shader.hpp"
#include "text.hpp"
#include "matrix.hpp"

Renderer::Renderer(const Matrix4& projection)
	: shapeShader("shape", "res/shape.vert", "res/shape.frag"),
	  textureShader("texture", "res/texture.vert", "res/texture.frag"),
	  currentFont(nullptr)
{
	updateProjection(projection);
}

void Renderer::updateProjection(const Matrix4& projection)
{
	GLint projUniformLocation = -1;

	glUseProgram(shapeShader.programID);
	projUniformLocation = glGetUniformLocation(shapeShader.programID, "projection");

	if (projUniformLocation != -1)
	{
		glUniformMatrix4fv(projUniformLocation, 1, false, projection.data);
	}

	glUseProgram(textureShader.programID);
	projUniformLocation = glGetUniformLocation(textureShader.programID, "projection");

	if (projUniformLocation != -1)
	{
		glUniformMatrix4fv(projUniformLocation, 1, false, projection.data);
	}
}

void Renderer::drawRect(float x, float y, float width, float height)
{
	glUseProgram(shapeShader.programID);

	// TODO(fkp): Should these be member variables
	static GLuint vao = 0;
	static GLuint vbo = 0;

	if (vao == 0)
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	}

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	Vertex vertices[] = {
		{ x,         y,          0.0, 1.0 },
		{ x + width, y,          0.0, 1.0 },
		{ x,         y + height, 0.0, 1.0 },
		{ x + width, y,          0.0, 1.0 },
		{ x,         y + height, 0.0, 1.0 },
		{ x + width, y + height, 0.0, 1.0 },
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::drawText(const std::string& text, unsigned int messageLength, float x, float y, float wrapWidth)
{
	if (!currentFont)
	{
		static bool hasWarned = false;

		if (!hasWarned)
		{
			printf("Error: No font selected.\n");
			return;
		}
	}
	
	glUseProgram(textureShader.programID);
	glUniform4f(glGetUniformLocation(textureShader.programID, "textColour"), 1.0f, 1.0f, 1.0f, 1.0f);

	// For ease of use (arrows are dumb)
	const Font& font = *currentFont;

	// Warns about a wrapWidth that is too small
	if (wrapWidth != 0.0 && wrapWidth < (float) font.maxGlyphAdvanceX)
	{
		// TODO(fkp): Warn once macro
		static bool hasWarned = false;

		if (!hasWarned)
		{
			hasWarned = true;
			printf("Warning: Wrap width smaller than glyph.\n");

			return;
		}
	}

	glBindVertexArray(font.vao);
	glBindTexture(GL_TEXTURE_2D, font.textureAtlas);

	// Set up the VBO
	glBindBuffer(GL_ARRAY_BUFFER, font.vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

	// Creates a buffer for all the vertices in the text
	unsigned int verticesBufferSize = 6 * text.size();
	Vertex* vertices = new Vertex[verticesBufferSize];
	int count = 0;

	float startX = x;
	int loopCounter = 0;

	// Loop through every character (until message length (if provided))
	for (const char& currentChar : text)
	{
		// Breaks after enough characters
		if (messageLength != -1 && loopCounter++ >= messageLength)
		{
			break;
		}

		// Next line
		if (currentChar == '\n' ||
			(wrapWidth != 0.0f && x + font.chars[currentChar].advanceX > wrapWidth))
		{
			x = startX;
			y += font.size;
		}

		// Calculates vertex dimensions
		float coordX = x + font.chars[currentChar].bitmapLeft;
		float coordY = y + font.maxGlyphBearingY - font.chars[currentChar].bitmapTop;
		float width = font.chars[currentChar].bitmapWidth;
		float height = font.chars[currentChar].bitmapHeight;

		// Advance the cursor to the next character
		x += font.chars[currentChar].advanceX;
		y += font.chars[currentChar].advanceY;

		// Skip glyphs with no pixels
		if (!width || !height)
		{
			continue;
		}

		// Calculates texture dimensions
		float bitmapX = font.chars[currentChar].textureX;
		float bitmapY = font.chars[currentChar].textureY;
		float bitmapWidth = font.chars[currentChar].bitmapWidth;
		float bitmapHeight = font.chars[currentChar].bitmapHeight;

		// Vertex 0
		vertices[count++] = Vertex {
			coordX,
			coordY,
			bitmapX,
			bitmapY
		};

		// Vertex 1
		vertices[count++] = Vertex {
			coordX + width,
			coordY,
			bitmapX + (bitmapWidth / font.textureAtlasWidth),
			bitmapY
		};

		// Vertex 2
		vertices[count++] = Vertex {
			coordX,
			coordY + height,
			bitmapX,
			bitmapY + (bitmapHeight / font.textureAtlasHeight)
		};

		// Vertex 1
		vertices[count++] = Vertex {
			coordX + width,
			coordY,
			bitmapX + (bitmapWidth / font.textureAtlasWidth),
			bitmapY
		};

		// Vertex 2
		vertices[count++] = Vertex {
			coordX,
			coordY + height,
			bitmapX,
			bitmapY + (bitmapHeight / font.textureAtlasHeight)
		};

		// Vertex 3
		vertices[count++] = Vertex {
			coordX + width,
			coordY + height,
			bitmapX + (bitmapWidth / font.textureAtlasWidth),
			bitmapY + (bitmapHeight / font.textureAtlasHeight)
		};
	}

	// Draw the text
	glBufferData(GL_ARRAY_BUFFER, verticesBufferSize * sizeof(Vertex), vertices, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, count);

	delete[] vertices;
}