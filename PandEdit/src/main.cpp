//  ===== Date Created: 14 April, 2020 ===== 

#include <stdio.h>
#include <windows.h>
#include <glad/glad.h>

#include "window.hpp"

int main(int argc, char* argv[])
{
	Window window { 960, 540, "PandEdit" };
	glClearColor(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 255.0f);

	while (window.isOpen)
	{
		MSG message;
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		SwapBuffers(window.deviceContext);
	}
	
	return 0;
}
