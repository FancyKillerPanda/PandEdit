//  ===== Date Created: 14 April, 2020 ===== 

#include <ft2build.h>
#include FT_FREETYPE_H

#include "text.hpp"

#define MAX_TEXTURE_ATLAS_WIDTH 512

Font::Font(std::string name, const char* path, unsigned int size)
	: size(size), name(name)
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);

	// FreeType initialisation
	FT_Library freetypeLibrary;

	if (FT_Init_FreeType(&freetypeLibrary) != 0)
	{
		printf("Error: Failed to initialise FreeType.\n");
		return;
	}

	// Loads the font
	FT_Face fontFace;

	if (FT_New_Face(freetypeLibrary, path, 0, &fontFace) != 0)
	{
		printf("Error: Failed to create font face for '%s'.\n", path);
		return;
	}

	FT_Set_Pixel_Sizes(fontFace, 0, size);
	FT_GlyphSlot glyph = fontFace->glyph; // For ease of use

	// Texture dimensions
	unsigned int currentWidth = 0;
	unsigned int currentHeight = 0;

	// TODO(fkp): Non-ASCII characters
	for (unsigned char i = 32; i < 128; i++)
	{
		if (FT_Load_Char(fontFace, i, FT_LOAD_RENDER) != 0)
		{
			printf("Error: Failed to load character '%c'.\n", i);
			continue;
		}

		// Don't make the texture atlas wider than a set amount
		if (currentWidth + glyph->bitmap.width + 1 > MAX_TEXTURE_ATLAS_WIDTH)
		{
			if (currentWidth > textureAtlasWidth) textureAtlasWidth = currentWidth;
			textureAtlasHeight += currentHeight;

			currentWidth = 0;
			currentHeight = 0;
		}

		currentWidth += glyph->bitmap.width + 1;
		if (glyph->bitmap.rows > currentHeight) currentHeight = glyph->bitmap.rows;
		if (glyph->bitmap_top > (int) maxGlyphBearingY) maxGlyphBearingY = glyph->bitmap_top;

		if ((unsigned int) (glyph->advance.x >> 6) > maxGlyphAdvanceX)
		{
			maxGlyphAdvanceX = (unsigned int) (glyph->advance.x >> 6);
		}
	}

	if (currentWidth > textureAtlasWidth) textureAtlasWidth = currentWidth;
	textureAtlasHeight += currentHeight;

	// Create the texture atlas
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &textureAtlas);
	glBindTexture(GL_TEXTURE_2D, textureAtlas);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, textureAtlasWidth, textureAtlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Pastes the glyphs into the texture
	int offsetX = 0;
	int offsetY = 0;
	currentHeight = 0;

	for (unsigned char i = 32; i < 128; i++)
	{
		if (FT_Load_Char(fontFace, i, FT_LOAD_RENDER) != 0)
		{
			continue;
		}

		if (offsetX + glyph->bitmap.width + 1 > MAX_TEXTURE_ATLAS_WIDTH)
		{
			offsetX = 0;
			offsetY += currentHeight;
			currentHeight = 0;
		}

		if (glyph->bitmap.width != 0 && glyph->bitmap.rows != 0)
		{
			glTexSubImage2D(GL_TEXTURE_2D, 0, offsetX, offsetY, glyph->bitmap.width, glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer);
		}

		// Sets character data
		chars[i].advanceX = (float) (glyph->advance.x >> 6);
		chars[i].advanceY = (float) (glyph->advance.y >> 6);

		chars[i].bitmapLeft = (float) glyph->bitmap_left;
		chars[i].bitmapTop = (float) glyph->bitmap_top;
		chars[i].bitmapWidth = (float) glyph->bitmap.width;
		chars[i].bitmapHeight = (float) glyph->bitmap.rows;

		chars[i].textureX = (float) offsetX / (float) textureAtlasWidth;
		chars[i].textureY = (float) offsetY / (float) textureAtlasHeight;

		if (glyph->bitmap.rows > currentHeight) currentHeight = glyph->bitmap.rows;
		offsetX += glyph->bitmap.width + 1;
	}

	fontsMap.insert({ name, this });
	
	printf("Info: Generated a %dx%d (%dkB) texture for '%s'.\n", textureAtlasWidth, textureAtlasHeight, (textureAtlasWidth * textureAtlasHeight) / 1024, path);

	// Cleanup
	FT_Done_Face(fontFace);
	FT_Done_FreeType(freetypeLibrary);
}

Font::~Font()
{
	fontsMap.erase(name);
}

Font* Font::get(const std::string& name)
{
	auto result = fontsMap.find(name);

	if (result != fontsMap.end())
	{
		return result->second;
	}
	else
	{
		return nullptr;
	}
}
