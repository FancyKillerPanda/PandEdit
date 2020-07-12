//  ===== Date Created: 14 April, 2020 =====

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>

#include <glad/glad.h>

#include "window.hpp"
#include "shader.hpp"
#include "renderer.hpp"
#include "font.hpp"
#include "default_key_bindings.hpp"
#include "timer.hpp"
#include "colour.hpp"

int main(int argc, char* argv[])
{
	Window window { 960, 540, "PandEdit" };

	std::vector<std::string> args(argv, argv + argc);
	window.parseArguments(std::move(args));

	glClearColor(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 255.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Font arialFont("arial", "res/arial.ttf", 30);
	Font consolasFont16("consolas-15", "res/consola.ttf", 15);
	Font consolasFont24("consolas-24", "res/consola.ttf", 24);
	window.setFont(&consolasFont24);

	mapDefaultKeyBindings();	

	std::string fpsTextString = "0ms";
	Timer fpsTimer;
	unsigned int numberOfFrames = 0;
	
	TextToDraw fpsText { fpsTextString };
	fpsText.startX = window.width - 20;
	fpsText.colour = getDefaultTextColour();
	fpsText.rightAlign = true;
	
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

		double frameTime = fpsTimer.getElapsedMs();
		
		if (frameTime > 1000.0)
		{			
			constexpr unsigned int bufferSize = 16;
			char buffer[bufferSize];
			snprintf(buffer, bufferSize, "%.2fms", frameTime / numberOfFrames);
				
			fpsTextString = buffer;
			fpsTimer.reset();
			fpsText.textLength = fpsTextString.size();
			
			numberOfFrames = 0;
		}
		
		numberOfFrames += 1;
		window.renderer->drawText(fpsText);
		
		// TODO(fkp): Figure out how to avoid doing this. If the
		// TextToDraw object is persistent across frames, the x value
		// increases continually.
		fpsText.startX = window.width - 20;
		fpsText.x = fpsText.startX;
		
		SwapBuffers(window.deviceContext);
	}

	return 0;
}
