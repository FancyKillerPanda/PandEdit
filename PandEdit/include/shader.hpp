//  ===== Date Created: 14 April, 2020 ===== 

#if !defined(SHADER_HPP)
#define SHADER_HPP

#include <string>
#include <unordered_map>
#include <glad/glad.h>

class Shader
{
public:
	std::string name;
	GLuint programID;

private:
	inline static std::unordered_map<std::string, Shader*> shadersMap;
	
public:
	Shader(std::string name, const char* vertexPath, const char* fragmentPath);
	~Shader();
	static Shader* get(const std::string& shaderName);

private:
	GLuint compileShader(GLenum type, const char* source);
	void destroyShader(GLuint shader);
};

#endif
