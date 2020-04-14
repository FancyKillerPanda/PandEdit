//  ===== Date Created: 14 April, 2020 ===== 

#if !defined(SHADER_HPP)
#define SHADER_HPP

#include <string>
#include <glad/glad.h>

class Shader
{
public:
	std::string name;
	GLuint programID;

public:
	Shader(std::string name, const char* vertexPath, const char* fragmentPath);

private:
	GLuint compileShader(GLenum type, const char* source);
	void destroyShader(GLuint shader);
};

#endif
