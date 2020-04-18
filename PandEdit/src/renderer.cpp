//  ===== Date Created: 14 April, 2020 =====

#include "renderer.hpp"
#include "shader.hpp"
#include "text.hpp"
#include "matrix.hpp"
#include "frame.hpp"

Renderer::Renderer(const Matrix4& projection, float windowWidth, float windowHeight)
	: shapeShader("shape", "res/shape.vert", "res/shape.frag"),
	  textureShader("texture", "res/texture.vert", "res/texture.frag"),
	  currentFont(nullptr)
{
	updateShaderUniforms(projection, windowWidth, windowHeight);
}

// TODO(fkp): This method has a lot of code duplication
void Renderer::updateShaderUniforms(const Matrix4& projection, float windowWidth, float windowHeight)
{
	GLint projUniformLocation = -1;
	GLint resolutionUniformLocation = -1;

	glUseProgram(shapeShader.programID);
	projUniformLocation = glGetUniformLocation(shapeShader.programID, "projection");

	if (projUniformLocation != -1)
	{
		glUniformMatrix4fv(projUniformLocation, 1, false, projection.data);
	}

	resolutionUniformLocation = glGetUniformLocation(shapeShader.programID, "resolution");

	if (resolutionUniformLocation != -1)
	{
		glUniform2f(resolutionUniformLocation, windowWidth, windowHeight);
	}

	glUseProgram(textureShader.programID);
	projUniformLocation = glGetUniformLocation(textureShader.programID, "projection");

	if (projUniformLocation != -1)
	{
		glUniformMatrix4fv(projUniformLocation, 1, false, projection.data);
	}

	resolutionUniformLocation = glGetUniformLocation(textureShader.programID, "resolution");

	if (resolutionUniformLocation != -1)
	{
		glUniform2f(resolutionUniformLocation, windowWidth, windowHeight);
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

void Renderer::drawHollowRect(float x, float y, float width, float height, float borderWidth)
{
	glUseProgram(shapeShader.programID);
	glUniform1f(glGetUniformLocation(shapeShader.programID, "borderWidth"), borderWidth);
	glUniform4f(glGetUniformLocation(shapeShader.programID, "rectangleDimensions"), x, y, width, height);

	drawRect(x, y, width, height);
	glUniform1f(glGetUniformLocation(shapeShader.programID, "borderWidth"), 0.0f);
}

void Renderer::drawText(const std::string& text, int messageLength, float x, float y, float wrapWidth)
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

	// TODO(fkp): Set if not set
	// glUniform4f(glGetUniformLocation(textureShader.programID, "textColour"), 1.0f, 1.0f, 1.0f, 1.0f);

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
			(wrapWidth != 0.0f && x + font.chars[currentChar].advanceX > startX + wrapWidth))
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

void Renderer::drawFrame(Frame& frame)
{
	//
	// Text
	//

	glUseProgram(textureShader.programID);
	glUniform4f(glGetUniformLocation(textureShader.programID, "textColour"), 1.0f, 1.0f, 1.0f, 1.0f);

	Buffer& buffer = *frame.currentBuffer;
	int y = frame.y;

	for (const std::string& line : buffer.data)
	{
		drawText(line, -1, frame.x, y, frame.width);
		y += currentFont->size;
	}

	//
	// Point
	//

	float pointX = frame.x;
	float pointY = frame.y + (buffer.line * currentFont->size);
	float pointWidth;
	float pointHeight = (float) currentFont->size;

	for (unsigned int i = 0; i < buffer.col; i++)
	{
		const Character& character = currentFont->chars[buffer.data[buffer.line][i]];
		pointX += character.advanceX;
	}

	if (buffer.col == buffer.data[buffer.line].size())
	{
		pointWidth = (float) currentFont->maxGlyphAdvanceX;
	}
	else
	{
		pointWidth = currentFont->chars[buffer.data[buffer.line][buffer.col]].advanceX;
	}

	glUseProgram(shapeShader.programID);
	glUniform4f(glGetUniformLocation(shapeShader.programID, "colour"), 1.0f, 1.0f, 1.0f, 1.0f);

	if (&frame == Frame::currentFrame)
	{
		if (buffer.pointFlashFrameCounter++ % 90 < 45)
		{
			drawRect(pointX, pointY, pointWidth, pointHeight);
		}
	}
	else
	{
		if (buffer.type != BufferType::MiniBuffer)
		{
			unsigned int borderWidth = 1 + (currentFont->size / 24);
			drawHollowRect(pointX, pointY, pointWidth, pointHeight, (float) borderWidth);
		}
	}

	//
	// Border
	//

	glUseProgram(shapeShader.programID);
	glUniform4f(glGetUniformLocation(shapeShader.programID, "colour"), 0.2f, 0.2f, 0.3f, 1.0f);

	drawRect(frame.realX, frame.y, FRAME_BORDER_WIDTH, frame.height);
	drawRect(frame.realX + frame.realWidth - FRAME_BORDER_WIDTH, frame.y, FRAME_BORDER_WIDTH, frame.height);

	//
	// Mode line
	//

	if (buffer.type != BufferType::MiniBuffer)
	{
		glUseProgram(shapeShader.programID);

		if (&frame == Frame::currentFrame)
		{
			glUniform4f(glGetUniformLocation(shapeShader.programID, "colour"), 1.0f, 1.0f, 1.0f, 1.0f);
		}
		else
		{
			glUniform4f(glGetUniformLocation(shapeShader.programID, "colour"), 0.2f, 0.2f, 0.2f, 1.0f);
		}

		drawRect(frame.realX, frame.y + frame.height - currentFont->size, frame.realWidth, currentFont->size);
		glUseProgram(textureShader.programID);

		if (&frame == Frame::currentFrame)
		{
			glUniform4f(glGetUniformLocation(textureShader.programID, "textColour"), 0.2f, 0.2f, 0.2f, 1.0f);
		}
		else
		{
			glUniform4f(glGetUniformLocation(textureShader.programID, "textColour"), 1.0f, 1.0f, 1.0f, 1.0f);
		}

		std::string modeLineText = buffer.name;
		modeLineText += " (LINE: ";
		modeLineText += std::to_string(buffer.line + 1);
		modeLineText += ", COL: ";
		modeLineText += std::to_string(buffer.col);
		modeLineText += ")";

		drawText(modeLineText, modeLineText.size(), frame.x, frame.y + frame.height - currentFont->size, frame.width);
	}
}
