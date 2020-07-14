//  ===== Date Created: 14 April, 2020 =====

#include "renderer.hpp"
#include "shader.hpp"
#include "font.hpp"
#include "matrix.hpp"
#include "frame.hpp"
#include "common.hpp"
#include "lexer.hpp"
#include "colour.hpp"

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
	glUniform4f(glGetUniformLocation(textureShader.programID, "textColour"), textToDraw.colour.r, textToDraw.colour.g, textToDraw.colour.b, textToDraw.colour.a);

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
	unsigned int maxLineWidth = 0;

	// Loop through every character (until message length (if provided))
	for (int i = 0; i < textToDraw.text.size(); i++)
	{
		unsigned char currentChar = textToDraw.text[i];
		
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
			advanceToNextTabStop(tabWidth, &font, textToDraw.x, textToDraw.numberOfColumnsInLine);			
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

		// Adds to the line width
		if (textToDraw.x - textToDraw.startX > maxLineWidth)
		{
			maxLineWidth = textToDraw.x - textToDraw.startX;
		}
		
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

	// Adjusts x-position if we are supposed to be right aligned
	if (textToDraw.rightAlign)
	{
		for (int i = 0; i < count; i++)
		{
			vertices[i].x -= maxLineWidth;
		}
	}
	
	// Draw the text
	glBufferData(GL_ARRAY_BUFFER, verticesBufferSize * sizeof(Vertex), vertices, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, count);

	delete[] vertices;
	return;
}

void updateTopLine(int& currentTopLine, int targetTopLine, unsigned int numberOfLinesInView, unsigned int totalNumberOfLines)
{
	// TODO(fkp): This is dependent on frame rate
	if (currentTopLine < targetTopLine)
	{
		currentTopLine += 2;

		// If on the other side now
		if (currentTopLine > targetTopLine)
		{
			currentTopLine = targetTopLine;
		}
	}
	else if (currentTopLine > targetTopLine)
	{
		currentTopLine -= 2;

		// If on the other side now
		if (currentTopLine < targetTopLine)
		{
			currentTopLine = targetTopLine;
		}
	}

	if (currentTopLine < targetTopLine &&
		targetTopLine - currentTopLine > numberOfLinesInView)
	{
		currentTopLine = targetTopLine - numberOfLinesInView;
	}
	else if (currentTopLine > targetTopLine &&
			 currentTopLine - targetTopLine > numberOfLinesInView)
	{
		currentTopLine = targetTopLine + numberOfLinesInView;
	}

	if (currentTopLine > (int) totalNumberOfLines - 2)
	{
		currentTopLine = (int) totalNumberOfLines - 2;
	}

	// Don't make this an else if
	if (currentTopLine < 0)
	{
		currentTopLine = 0;
	}	
}

void Renderer::drawFrame(Frame& frame)
{
	if (frame.childOne != nullptr || frame.childTwo != nullptr)
	{
		// Frame is solely for grouping
		return;
	}
	
	Buffer& buffer = *frame.currentBuffer;
	updateTopLine(frame.currentTopLine, frame.targetTopLine, frame.numberOfLinesInView, buffer.data.size());

	//
	// Frame and point rects
	//

	int realFramePixelX;
	unsigned int realFramePixelWidth;
	int framePixelX;
	int framePixelY;
	unsigned int framePixelWidth;
	unsigned int framePixelHeight;
	frame.getRect(currentFont, &realFramePixelX, &realFramePixelWidth, &framePixelX, &framePixelY, &framePixelWidth, &framePixelHeight);

	float pointX;
	float pointY;
	float pointWidth;
	float pointHeight;
	frame.getPointRect(currentFont, tabWidth, framePixelX, framePixelY, &pointX, &pointY, &pointWidth, &pointHeight);
	
	//
	// Highlighting the current line
	//
	
	if (&frame == Frame::currentFrame &&
		buffer.type != BufferType::MiniBuffer &&
		pointX + pointWidth <= framePixelX + framePixelWidth &&
		pointY >= framePixelY &&
		pointY + pointHeight <= framePixelY + framePixelHeight - currentFont->size)
	{
		glUseProgram(shapeShader.programID);
		glUniform4f(glGetUniformLocation(shapeShader.programID, "colour"), 0.19f, 0.19f, 0.19f, 1.0f);

		drawRect(realFramePixelX + FRAME_BORDER_WIDTH, pointY, realFramePixelWidth - FRAME_BORDER_WIDTH, pointHeight);
	}

	//
	// Text
	//

	// TODO(fkp): Uniform setters for colours
	Colour defaultColour = getDefaultTextColour();
	
	int y = framePixelY;
	std::string visibleLines = "";
	unsigned int numberOfLines = 0;

	for (unsigned int i = frame.currentTopLine; i < buffer.data.size(); i++)
	{
		if (y + currentFont->size > framePixelY + framePixelHeight)
		{
			break;
		}

		visibleLines += buffer.data[i] + std::string(1, '\n');
		y += currentFont->size;
		numberOfLines += 1;
	}

	std::vector<Token*> bufferTokens;
	
	if (buffer.isUsingSyntaxHighlighting)
	{
		bufferTokens = buffer.lexer.getTokens(frame.currentTopLine, frame.currentTopLine + numberOfLines - 1);
	}

	if (bufferTokens.size() == 0)
	{
		TextToDraw textToDraw { visibleLines };
		textToDraw.textLength = visibleLines.size();
		textToDraw.x = framePixelX;
		textToDraw.y = framePixelY;
		textToDraw.maxWidth = framePixelWidth;
		textToDraw.colour = defaultColour;
		
		drawText(textToDraw);
	}
	else
	{
		Point lastTokenEnd { bufferTokens[0]->start.buffer };
		lastTokenEnd.line = frame.currentTopLine;
		
		std::string textToDrawString = "";
		TextToDraw textToDraw { textToDrawString };
		textToDraw.startX = framePixelX;
		textToDraw.x = framePixelX;
		textToDraw.y = framePixelY;
		textToDraw.maxWidth = framePixelWidth;
		textToDraw.colour = defaultColour;

		Point stringEndPoint = getPointAtEndOfString(visibleLines, frame.currentTopLine);

		for (int i = 0; i < bufferTokens.size(); i++)
		{
			const Token& token = *bufferTokens[i];
			
			if (token.start > stringEndPoint)
			{
				break;
			}

			if (token.end < Point { (unsigned int) frame.currentTopLine, 0 })
			{
				continue;
			}
			
			Point tokenStart = token.start;
			
			if (token.start < Point { (unsigned int) frame.currentTopLine, 0 })
			{
				tokenStart.line = frame.currentTopLine;
				tokenStart.col = 0;
			}
						
			if (tokenStart > lastTokenEnd)
			{
				textToDraw.colour = defaultColour;
				textToDrawString = substrFromPoints(visibleLines, lastTokenEnd, tokenStart, frame.currentTopLine);
				
				 drawText(textToDraw);
			}

			lastTokenEnd = token.end;
			textToDraw.colour = getColourForTokenType(token.type);
			textToDrawString = substrFromPoints(visibleLines, tokenStart, token.end, frame.currentTopLine);
			
			drawText(textToDraw);
		}

		// Draws the rest of the text if needed
		if (lastTokenEnd < stringEndPoint)
		{
			textToDraw.colour = defaultColour;
			textToDrawString = substrFromPoints(visibleLines, lastTokenEnd, stringEndPoint, frame.currentTopLine);
			
			drawText(textToDraw);
		}
	}
	
	//
	// Point
	//

	if (frame.currentBuffer->type == BufferType::MiniBuffer ||
		(pointX + pointWidth <= framePixelX + framePixelWidth &&
		 pointY >= framePixelY &&
		 pointY + pointHeight <= framePixelY + framePixelHeight - currentFont->size))
	{
		glUseProgram(shapeShader.programID);
		glUniform4f(glGetUniformLocation(shapeShader.programID, "colour"), 1.0f, 1.0f, 1.0f, 1.0f);

		if (frame.overwriteMode)
		{
			pointY += pointHeight * 0.85f;
			pointHeight *= 0.15f;
		}
		
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

		std::string modeLineTextString = buffer.name;
		
		if (!frame.currentBuffer->isReadOnly &&
			frame.currentBuffer->numberOfActionsSinceSave > 0)
		{
			modeLineTextString += " (*)";
		}
		
		modeLineTextString += " (LINE: ";
		modeLineTextString += std::to_string(frame.point.line + 1);
		modeLineTextString += ", COL: ";
		modeLineTextString += std::to_string(frame.point.col);
		modeLineTextString += ")";

		TextToDraw modeLineText { modeLineTextString };
		
		if (&frame == Frame::currentFrame)
		{
			modeLineText.colour = Colour { 0.2f, 0.2f, 0.2f, 1.0f };
		}
		else
		{
			modeLineText.colour = Colour { 1.0f, 1.0f, 1.0f, 1.0f };
		}

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

void Renderer::drawFramePopups(Frame& frame)
{
#define NUM_CHAR_SPACE_BEFORE_INFO 2
	
	if (&frame == Frame::currentFrame && frame.popupLines.size() > 0)
	{
		// Gets the rects
		int realFramePixelX;
		unsigned int realFramePixelWidth;
		int framePixelX;
		int framePixelY;
		unsigned int framePixelHeight;
		frame.getRect(currentFont, &realFramePixelX, &realFramePixelWidth, &framePixelX, &framePixelY, nullptr, &framePixelHeight);

		float pointX;
		float pointY;
		float pointWidth;
		float pointHeight;
		frame.getPointRect(currentFont, tabWidth, framePixelX, framePixelY, &pointX, &pointY, &pointWidth, &pointHeight);
	
		// The number of lines in view for the popup
		unsigned int numberOfLines = frame.popupLines.size();
		if (numberOfLines > popupMaxNumberOfLines) numberOfLines = popupMaxNumberOfLines;

		// Workaround because a weird scrolling animation occurs when we wrap around
		if (frame.popupTargetTopLine == frame.popupLines.size() - (numberOfLines / 2) - 1 &&
			(frame.popupCurrentTopLine == 0 || frame.popupCurrentTopLine == 1))
		{
			frame.popupCurrentTopLine = frame.popupLines.size();
		}
		else if ((frame.popupTargetTopLine == 0 || frame.popupTargetTopLine == 1) &&
				 frame.popupCurrentTopLine == frame.popupLines.size() - (numberOfLines / 2) - 1)
		{
			frame.popupCurrentTopLine = 1 - ((int) numberOfLines / 2);
		}
		
		updateTopLine(frame.popupCurrentTopLine, frame.popupTargetTopLine, numberOfLines, frame.popupLines.size());

		// Figures out the dimensions for the popup
		int popupX = pointX + (pointWidth * 1.5f);
		int popupY = pointY;
		unsigned int popupWidth;
		unsigned int popupHeight = currentFont->size * numberOfLines;

		unsigned int maxNumberOfChars = 0;
		
		for (int i = 0; i < numberOfLines; i++)
		{
			const std::pair<std::string, std::string>& line = frame.popupLines[i];
			unsigned int numberOfChars = getMaxWidthOfText(line.first) + NUM_CHAR_SPACE_BEFORE_INFO + 1 + getMaxWidthOfText(line.second);

			if (numberOfChars > maxNumberOfChars)
			{
				maxNumberOfChars = numberOfChars;
			}
		}

		popupWidth = maxNumberOfChars * currentFont->chars[(unsigned int) ' '].advanceX;
		
		if (popupX + popupWidth > realFramePixelX + realFramePixelWidth)
		{
			popupX -= popupWidth;
			popupY += currentFont->size;
		}

		if (popupX < 0)
		{
			popupX = 0;
		}
		
		if (popupY + popupHeight > framePixelY + framePixelHeight)
		{
			// Doesn't fit on the bottom of the screen
			if (popupY == pointY)
			{				
				popupY -= popupHeight;
				popupY += currentFont->size;
			}
			else
			{
				popupY -= popupHeight;
				popupY -= currentFont->size;
			}
		}

		// Draws the background
		// TODO(fkp): Try fit on the other side of the point
		glUseProgram(shapeShader.programID);
		glUniform4f(glGetUniformLocation(shapeShader.programID, "colour"), 0.0f, 0.0f, 0.0f, 1.0f);
		drawRect(popupX, popupY, popupWidth, popupHeight);
		glUniform4f(glGetUniformLocation(shapeShader.programID, "colour"), 0.4f, 0.4f, 0.4f, 1.0f);
		drawHollowRect(popupX, popupY, popupWidth, popupHeight, 1 + (pointHeight / 24));

		// Draws the text
		std::string text;
		TextToDraw textToDraw { text };
		textToDraw.x = popupX + (pointWidth / 2);
		textToDraw.y = popupY;
		textToDraw.maxWidth = popupWidth - pointWidth;
		textToDraw.colour = getDefaultTextColour();

		Colour infoColour = normaliseColour(127, 127, 127, 255);
		int selectionListIndex = -1;

		for (int i = 0; i < numberOfLines; i++)
		{
			int lineIndex = frame.popupCurrentTopLine + i;

			if (lineIndex >= frame.popupLines.size())
			{
				lineIndex -= frame.popupLines.size();
			}

			if (lineIndex == frame.popupCurrentSuggestion)
			{
				selectionListIndex = i;
			}
			
			// Main suggestion
			textToDraw.colour = getDefaultTextColour();			
			text = frame.popupLines[lineIndex].first;
			drawText(textToDraw);

			// Adds some spacing
			textToDraw.x += currentFont->chars[(unsigned char) ' '].advanceX * NUM_CHAR_SPACE_BEFORE_INFO;

			// Additional information
			textToDraw.colour = infoColour;
			text = frame.popupLines[lineIndex].second;
			text += std::string(1, '\n');
			drawText(textToDraw);
		}
		
		// Draws the highlight box
		if (selectionListIndex != -1)
		{
			glUseProgram(shapeShader.programID);
			glUniform4f(glGetUniformLocation(shapeShader.programID, "colour"), 0.5f, 0.5f, 0.8f, 1.0f);
			drawHollowRect(popupX, popupY + (selectionListIndex * pointHeight), popupWidth, pointHeight, 1 + (pointHeight / 24));
		}
	}

#undef NUM_CHAR_SPACE_BEFORE_INFO
}

unsigned int Renderer::getMaxWidthOfText(const std::string& text)
{
	unsigned int maxWidth = 0;
	unsigned int currentWidth = 0;

	for (const char& character : text)
	{
		if (character == '\n')
		{
			if (currentWidth > maxWidth)
			{
				maxWidth = currentWidth;
			}

			currentWidth = 0;
		}
		else if (character == '\t')
		{
			float _unused = 0.0f;
			advanceToNextTabStop(tabWidth, currentFont, _unused, currentWidth);
		}
		else
		{
			currentWidth += 1;
		}
	}

	if (currentWidth > maxWidth)
	{
		maxWidth = currentWidth;
	}

	return maxWidth;
}
