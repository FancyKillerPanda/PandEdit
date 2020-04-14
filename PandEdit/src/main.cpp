//  ===== Date Created: 14 April, 2020 ===== 

#include <stdio.h>
#include <windows.h>
#include <string>
#include <fstream>

#include <glad/glad.h>

#include "window.hpp"
#include "shader.hpp"
#include "matrix.hpp"
#include "renderer.hpp"

// TODO(fkp): Find somewhere better to put this
std::string readFile(const char* filename)
{
	std::ifstream file(filename);

	if (!file)
	{
		printf("Error: Failed to read file '%s'.\n", filename);
		return "";
	}

	file.seekg(0, std::ios::end);
	std::size_t size = file.tellg();
	file.seekg(0);

	std::string buffer(size, ' ');
	file.read(&buffer[0], size);

	return buffer;
}

int main(int argc, char* argv[])
{
	Window window { 960, 540, "PandEdit" };
	glClearColor(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 255.0f);

	Matrix4 projection = Matrix4::ortho(0, 960, 0, 540, -1, 1);
	Renderer renderer { projection };
	
	while (window.isOpen)
	{
		MSG message;
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderer.drawRect(50, 50, 100, 100);
		SwapBuffers(window.deviceContext);
	}
	
	return 0;
}
