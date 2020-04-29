//  ===== Date Created: 14 April, 2020 ===== 

#include "shader.hpp"
#include "file_util.hpp"

Shader::Shader(std::string name, const char* vertexPath, const char* fragmentPath)
	: name(name)
{
	std::string vertexSource = readFile(vertexPath);
	std::string fragmentSource = readFile(fragmentPath);
	
	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource.c_str());
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());

	// Don't try link program if shader compile failed
	if (vertexShader == 0 || fragmentShader == 0) return;
	
	programID = glCreateProgram();
	glAttachShader(programID, vertexShader);
	glAttachShader(programID, fragmentShader);
	glLinkProgram(programID);

	// Error checking
	GLint success;
	glGetProgramiv(programID, GL_LINK_STATUS, &success);

	if (!success)
	{
		GLint messageLength;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &messageLength);

		// Gets the message
		char* message = new char[messageLength];
		glGetProgramInfoLog(programID, messageLength, &messageLength, message);

		printf("Error: Program link failed (%s).\n", message);
		glDeleteProgram(programID);
		programID = 0;

		delete[] message;
	}
	else
	{
		shadersMap.insert({ name, this });
	}

	glDetachShader(programID, vertexShader);
	glDetachShader(programID, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
	shadersMap.erase(name);
}

Shader* Shader::get(const std::string& shaderName)
{
	// TODO(fkp): Caching?
	auto result = shadersMap.find(shaderName);

	if (result != shadersMap.end())
	{
		return result->second;
	}
	else
	{
		return nullptr;
	}
}

GLuint Shader::compileShader(GLenum type, const char* source)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	// Error checking
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		GLint messageLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &messageLength);

		// Gets the message
		char* message = new char[messageLength];
		glGetShaderInfoLog(shader, messageLength, &messageLength, message);

		printf("Error: %s shader compile failed (%s).\n", type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", message);
		glDeleteShader(shader);
		delete[] message;

		return 0;
	}

	return shader;
}
