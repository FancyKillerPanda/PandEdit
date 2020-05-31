//  ===== Date Created: 14 April, 2020 =====

#include "renderer.hpp"
#include "shader.hpp"
#include "font.hpp"
#include "matrix.hpp"
#include "frame.hpp"
#include "common.hpp"
#include "lexer.hpp"

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

void Renderer::drawText(TextToDraw& textToDraw)
{
	if (!currentFont)
	{
		ERROR_ONCE("Error: No font selected.\n");
		return;
	}

	glUseProgram(textureShader.programID);

	// TODO(fkp): Set if not set
	// glUniform4f(glGetUniformLocation(textureShader.programID, "textColour"), 1.0f, 1.0f, 1.0f, 1.0f);

	// For ease of use (arrows are dumb)
	const Font& font = *currentFont;

	// Warns about a max width that is too small
	if (textToDraw.maxWidth != 0.0f && textToDraw.maxWidth < (float) font.maxGlyphAdvanceX)
	{
		ERROR_ONCE("Warning: Max width smaller than glyph.\n");
		return;
	}

	// Warns about wrap set but no width provided
	if (textToDraw.wrap && textToDraw.maxWidth <= 0.0f)
	{
		ERROR_ONCE("Warning: Max width not provided but wrap requested.\n");
		return;
	}

	glBindVertexArray(font.vao);
	glBindTexture(GL_TEXTURE_2D, font.textureAtlas);

	// Set up the VBO
	glBindBuffer(GL_ARRAY_BUFFER, font.vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

	// Creates a buffer for all the vertices in the text
	unsigned int verticesBufferSize = 6 * textToDraw.text.size();
	Vertex* vertices = new Vertex[verticesBufferSize];
	int count = 0;

	if (textToDraw.startX == -1.0f)
	{
		textToDraw.startX = textToDraw.x;
	}

	int loopCounter = 0;

	// Loop through every character (until message length (if provided))
	for (int i = 0; i < textToDraw.text.size(); i++)
	{
		char currentChar = textToDraw.text[i];
		
		// Breaks after enough characters
		if (textToDraw.textLength != -1 && loopCounter++ >= textToDraw.textLength)
		{
			break;
		}

		// Newline
		if (currentChar == '\n')
		{
			textToDraw.x = textToDraw.startX;
			textToDraw.y += font.size;
			textToDraw.numberOfColumnsInLine = 0;

			continue;
		}
		
		// End of width
		if (textToDraw.maxWidth != 0.0f && textToDraw.x + font.chars[currentChar].advanceX > textToDraw.startX + textToDraw.maxWidth)
		{
			if (textToDraw.wrap)
			{
				textToDraw.x = textToDraw.startX;
				textToDraw.y += font.size;
				textToDraw.numberOfColumnsInLine = 0;

				continue;
			}
		}

		// Tabs
		if (currentChar == '\t')
		{
			// TODO(fkp): This doesn't work with non-monospaced fonts
			unsigned int numberOfColumnsToNextTabStop = tabWidth - (textToDraw.numberOfColumnsInLine % tabWidth);			
			textToDraw.numberOfColumnsInLine += numberOfColumnsToNextTabStop;

			textToDraw.x += font.chars[' '].advanceX * numberOfColumnsToNextTabStop;
			textToDraw.y += font.chars[' '].advanceY * numberOfColumnsToNextTabStop;
			
			continue;
		}

		// Calculates vertex dimensions
		float coordX = textToDraw.x + font.chars[currentChar].bitmapLeft;
		float coordY = textToDraw.y + font.maxGlyphBearingY - font.chars[currentChar].bitmapTop;
		float width = font.chars[currentChar].bitmapWidth;
		float height = font.chars[currentChar].bitmapHeight;

		// Advance the cursor to the next character
		textToDraw.x += font.chars[currentChar].advanceX;
		textToDraw.y += font.chars[currentChar].advanceY;
		textToDraw.numberOfColumnsInLine += 1;

		// Skip glyphs with no pixels
		if (!width || !height)
		{
			continue;
		}

		if (textToDraw.maxWidth != 0.0f && textToDraw.x > textToDraw.startX + textToDraw.maxWidth && !textToDraw.wrap)
		{
			// The wrap part shouldn't matter as it was checked earlier
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
	return;
}

// TODO(fkp): Find a better spot for this
std::string substrFromPoints(const std::string& string, const Point& start, const Point& end, unsigned int lineOffset)
{
	if (start > end)
	{
		ERROR_ONCE("Error: Start cannot be after end in substr.\n");
		return "";
	}

	if (lineOffset > start.line)
	{
		ERROR_ONCE("Error: String starts after the requested start point in substr.\n");
		return "";
	}
	
	unsigned int startIndex = 0;
	unsigned int currentIndex = 0;
	unsigned int linesPassed = lineOffset;
	
	while (currentIndex < string.size() && linesPassed < start.line)
	{
		if (string[currentIndex] == '\n')
		{
			linesPassed += 1;
		}

		currentIndex += 1;
	}

	currentIndex += start.col;
	startIndex = currentIndex;
	linesPassed = 0;
	unsigned int length = 0;

	while (currentIndex < string.size() && linesPassed < end.line - start.line)
	{
		if (string[currentIndex] == '\n')
		{
			linesPassed += 1;
		}

		length += 1;
		currentIndex += 1;
	}

	// The final line
	if (start.line == end.line)
	{
		length += end.col - start.col;
	}
	else
	{
		length += end.col;
	}

	return string.substr(startIndex, length);
}

// TODO(fkp): Find a better spot for this
Point getPointAtEndOfString(const std::string& string, unsigned int lineOffset)
{
	Point result;
	result.line = lineOffset;

	for (char character : string)
	{
		if (character == '\n')
		{
			result.line += 1;
			result.col = 0;
		}
		else
		{
			result.col += 1;
		}
	}

	return result;
}

void Renderer::drawFrame(Frame& frame)
{
	if (frame.childOne != nullptr || frame.childTwo != nullptr)
	{
		// Frame is solely for grouping
		return;
	}
	
	Buffer& buffer = *frame.currentBuffer;

	//
	// Pixel dimensions
	//
	
	int realFramePixelX = (int) (frame.pcDimensions.x * frame.windowWidth);
	unsigned int realFramePixelWidth = (unsigned int) (frame.pcDimensions.width * frame.windowWidth);
	int framePixelX = realFramePixelX + (FRAME_BORDER_WIDTH * 2);
	int framePixelY = (int) (frame.pcDimensions.y * frame.windowHeight);
	unsigned int framePixelWidth = realFramePixelWidth - (FRAME_BORDER_WIDTH * 2);
	unsigned int framePixelHeight = (unsigned int) (frame.pcDimensions.height * frame.windowHeight);

	if (frame.pcDimensions.y == 1.0f)
	{
		// This is the minibuffer
		framePixelHeight = currentFont->size;
	}
	
	//
	// Point dimensions
	//

	float pointX = framePixelX;
	float pointY = framePixelY + ((frame.point.line - frame.topLine) * currentFont->size);
	float pointWidth;
	float pointHeight = (float) currentFont->size;
	unsigned int numberOfColumnsInLine = 0;

	for (unsigned int i = 0; i < frame.point.col; i++)
	{
		const Character& character = currentFont->chars[buffer.data[frame.point.line][i]];

		if (buffer.data[frame.point.line][i] == '\n')
		{
			pointX = 0;
			pointY += currentFont->size;
			numberOfColumnsInLine = 0;
		}
		else if (buffer.data[frame.point.line][i] == '\t')
		{
			// NOTE(fkp): This is the same calculation as in the drawText() method
			unsigned int numberOfColumnsToNextTabStop = tabWidth - (numberOfColumnsInLine % tabWidth);
			pointX += currentFont->chars[' '].advanceX * numberOfColumnsToNextTabStop;
			numberOfColumnsInLine += numberOfColumnsToNextTabStop;
		}
		else
		{
			pointX += character.advanceX;
			numberOfColumnsInLine += 1;
		}
	}

	if (frame.point.col == buffer.data[frame.point.line].size())
	{
		pointWidth = (float) currentFont->maxGlyphAdvanceX;
	}
	else
	{
		char currentChar = buffer.data[frame.point.line][frame.point.col];

		if (currentChar == '\n' || currentChar == '\t')
		{
			pointWidth = currentFont->chars[' '].advanceX;
		}
		else
		{
			pointWidth = currentFont->chars[currentChar].advanceX;
		}
	}

	//
	// Highlighting the current line
	//
	
	if (&frame == Frame::currentFrame && buffer.type != BufferType::MiniBuffer)
	{
		glUseProgram(shapeShader.programID);
		glUniform4f(glGetUniformLocation(shapeShader.programID, "colour"), 0.19f, 0.19f, 0.19f, 1.0f);

		drawRect(realFramePixelX + FRAME_BORDER_WIDTH, pointY, realFramePixelWidth - FRAME_BORDER_WIDTH, pointHeight);
	}

	//
	// Text
	//

	// TODO(fkp): Uniform setters
	Colour defaultColour = getDefaultTextColour();
	glUseProgram(textureShader.programID);
	glUniform4f(glGetUniformLocation(textureShader.programID, "textColour"), defaultColour.r, defaultColour.g, defaultColour.b, defaultColour.a);

	int y = framePixelY;
	std::string visibleLines = "";
	unsigned int numberOfLines = 0;

	for (unsigned int i = frame.topLine; i < buffer.data.size(); i++)
	{
		if (y + currentFont->size > framePixelY + framePixelHeight)
		{
			break;
		}

		visibleLines += buffer.data[i] + std::string(1, '\n');
		y += currentFont->size;
		numberOfLines += 1;
	}

	std::vector<Token> bufferTokens;
	
	if (buffer.isUsingSyntaxHighlighting)
	{
		bufferTokens = buffer.lexer.getTokens(frame.topLine, frame.topLine + numberOfLines - 1);
	}

	if (bufferTokens.size() == 0)
	{
		TextToDraw textToDraw { visibleLines };
		textToDraw.textLength = visibleLines.size();
		textToDraw.x = framePixelX;
		textToDraw.y = framePixelY;
		textToDraw.maxWidth = framePixelWidth;
		
		drawText(textToDraw);
	}
	else
	{
		Point lastTokenEnd { bufferTokens[0].start.buffer };
		lastTokenEnd.line = frame.topLine;
		
		std::string textToDrawString = "";
		TextToDraw textToDraw { textToDrawString };
		textToDraw.startX = framePixelX;
		textToDraw.x = framePixelX;
		textToDraw.y = framePixelY;
		textToDraw.maxWidth = framePixelWidth;

		for (const Token& token : bufferTokens)
		{
			if (token.start > getPointAtEndOfString(visibleLines, frame.topLine))
			{
				break;
			}

			if (token.end < Point { frame.topLine, 0 })
			{
				continue;
			}
			
			Point tokenStart = token.start;
			
			if (token.start < Point { frame.topLine, 0 })
			{
				tokenStart.line = frame.topLine;
				tokenStart.col = 0;
			}
						
			if (tokenStart > lastTokenEnd)
			{
				glUniform4f(glGetUniformLocation(textureShader.programID, "textColour"), defaultColour.r, defaultColour.g, defaultColour.b, defaultColour.a);
				textToDrawString = substrFromPoints(visibleLines, lastTokenEnd, tokenStart, frame.topLine);
				
				 drawText(textToDraw);
			}

			lastTokenEnd = token.end;
			Colour textColour = getColourForTokenType(token.type);
			glUniform4f(glGetUniformLocation(textureShader.programID, "textColour"), textColour.r, textColour.g, textColour.b, textColour.a);
			textToDrawString = substrFromPoints(visibleLines, tokenStart, token.end, frame.topLine);
			
			drawText(textToDraw);
		}

		// Draws the rest of the text if needed
		Point stringEndPoint = getPointAtEndOfString(visibleLines, frame.topLine);
		
		if (lastTokenEnd < stringEndPoint)
		{
			glUniform4f(glGetUniformLocation(textureShader.programID, "textColour"), defaultColour.r, defaultColour.g, defaultColour.b, defaultColour.a);
			textToDrawString = substrFromPoints(visibleLines, lastTokenEnd, stringEndPoint, frame.topLine);
			
			drawText(textToDraw);
		}
	}
	
	//
	// Point
	//

	if (pointX + pointWidth <= framePixelX + framePixelWidth)
	{
		glUseProgram(shapeShader.programID);
		glUniform4f(glGetUniformLocation(shapeShader.programID, "colour"), 1.0f, 1.0f, 1.0f, 1.0f);

		if (&frame == Frame::currentFrame)
		{
			constexpr double FLASH_TIME = 600.0;
		
			if (frame.pointFlashTimer.getElapsedMs() < FLASH_TIME)
			{
				drawRect(pointX, pointY, pointWidth, pointHeight);
			}
			else if (frame.pointFlashTimer.getElapsedMs() > FLASH_TIME * 2)
			{
				frame.pointFlashTimer.reset();
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
	}

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

		drawRect(realFramePixelX, framePixelY + framePixelHeight - currentFont->size, realFramePixelWidth, currentFont->size);
		glUseProgram(textureShader.programID);

		if (&frame == Frame::currentFrame)
		{
			glUniform4f(glGetUniformLocation(textureShader.programID, "textColour"), 0.2f, 0.2f, 0.2f, 1.0f);
		}
		else
		{
			glUniform4f(glGetUniformLocation(textureShader.programID, "textColour"), 1.0f, 1.0f, 1.0f, 1.0f);
		}

		std::string modeLineTextString = buffer.name;
		modeLineTextString += " (LINE: ";
		modeLineTextString += std::to_string(frame.point.line + 1);
		modeLineTextString += ", COL: ";
		modeLineTextString += std::to_string(frame.point.col);
		modeLineTextString += ")";

		TextToDraw modeLineText { modeLineTextString };
		modeLineText.textLength = modeLineTextString.size();
		modeLineText.x = framePixelX;
		modeLineText.y = framePixelY + framePixelHeight - currentFont->size;
		modeLineText.maxWidth = framePixelWidth;
		
		drawText(modeLineText);
	}
	
	//
	// Border
	//

	glUseProgram(shapeShader.programID);
	glUniform4f(glGetUniformLocation(shapeShader.programID, "colour"), 0.2f, 0.2f, 0.3f, 1.0f);

	drawRect(realFramePixelX, framePixelY, FRAME_BORDER_WIDTH, framePixelHeight);
	drawRect(realFramePixelX + realFramePixelWidth - FRAME_BORDER_WIDTH, framePixelY, FRAME_BORDER_WIDTH, framePixelHeight);
}
