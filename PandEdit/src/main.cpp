//  ===== Date Created: 14 April, 2020 =====

#include <stdio.h>
#include <windows.h>
#include <string>
#include <fstream>
#include <vector>

#include <glad/glad.h>

#include "window.hpp"
#include "shader.hpp"
#include "renderer.hpp"
#include "font.hpp"
#include "default_key_bindings.hpp"

int main(int argc, char* argv[])
{
	Window window { 960, 540, "PandEdit" };

	std::vector<std::string> args(argv, argv + argc);
	window.parseArguments(std::move(args));

	glClearColor(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 255.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Font arialFont("arial", "res/arial.ttf", 30);
	Font consolasFont("consolas", "res/consola.ttf", 24);
	window.setFont(&consolasFont);

	mapDefaultKeyBindings();	
	
	while (window.isOpen)
	{
		MSG message;
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		window.draw();
		SwapBuffers(window.deviceContext);
	}

	return 0;
}
