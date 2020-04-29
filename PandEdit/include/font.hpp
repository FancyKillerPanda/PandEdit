//  ===== Date Created: 14 April, 2020 ===== 

#if !defined(TEXT_HPP)
#define TEXT_HPP

#include <string>
#include <unordered_map>

#include <glad/glad.h>

struct Character
{
	float advanceX = 0.0f;
	float advanceY = 0.0f;

	float bitmapLeft = 0.0f;
	float bitmapTop = 0.0f;
	float bitmapWidth = 0.0f;
	float bitmapHeight = 0.0f;

	float textureX = 0.0f;
	float textureY = 0.0f;
};

class Font
{
public:
	unsigned int size = 0;
	std::string name = "";
	
	GLuint vao = 0;
	GLuint vbo = 0;

	GLuint textureAtlas = 0;
	GLuint textureAtlasWidth = 0;
	GLuint textureAtlasHeight = 0;

	unsigned int maxGlyphAdvanceX = 0;
	unsigned int maxGlyphBearingY = 0;

	Character chars[128];

private:
	inline static std::unordered_map<std::string, Font*> fontsMap;

public:
	Font(std::string name, const char* path, unsigned int size);
	~Font();
	static Font* get(const std::string& name);
};

#endif
