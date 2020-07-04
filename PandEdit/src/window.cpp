//  ===== Date Created: 14 April, 2020 =====

#define PANDEDIT_DEBUG_WINDOW
#define PANDEDIT_VSYNC

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <windowsx.h> // For GET_X/Y_LPARAM macro
#include <filesystem>
#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include <glad/glad_wgl.h>

#include "window.hpp"
#include "renderer.hpp"
#include "matrix.hpp"
#include "font.hpp"
#include "commands.hpp"
#include "keymap.hpp"
#include "file_util.hpp"

Window::Window(unsigned int width, unsigned int height, const char* title)
	: isOpen(false),
	  width(width), height(height),
	  title(title), renderer(nullptr)
{
	if (registerWindowClass() &&
		createDummyWindow() &&
		loadWGLExtensions() &&
		destroyWindowComponents() &&
		createActualWindow())
	{
		registerDebugContextCallback();
		
		isOpen = true;
		windowsMap.insert({ windowHandle, this });

		Matrix4 projection = Matrix4::ortho(0, width, 0, height, -1, 1);
		renderer = new Renderer { projection, (float) width, (float) height };

		Frame::allFrames = &frames;
		frames.push_back(new Frame("mainFrame", Vector4f { 0.0f, 0.0f, 1.0f, 1.0f }, width, height, BufferType::Text, "*scratch*", true));
		frames.push_back(new Frame("minibufferFrame", Vector4f { 0.0f, 1.0f, 1.0f, 0.0f }, width, height, BufferType::MiniBuffer, "__minibuffer__", false));

		printf("Info: Created window (OpenGL: %s).\n", glGetString(GL_VERSION));
	}
}

Window::~Window()
{
	windowsMap.erase(windowHandle);
	destroyWindowComponents();
}

Window* Window::get(HWND handle)
{
	// TODO(fkp): Caching?
	auto result = windowsMap.find(handle);

	if (result != windowsMap.end())
	{
		return result->second;
	}
	else
	{
		return nullptr;
	}
}

LRESULT CALLBACK Window::eventCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
#define IS_KEY_PRESSED(key) (GetKeyState(key) & 0xFF00)

	// TODO(fkp): Only get this when needed?
	Window* window = Window::get(windowHandle);

	if (!window)
	{
		// This is *almost* certainly the dummy window
		return DefWindowProc(windowHandle, message, wParam, lParam);
	}

	// The current buffer active
	Buffer& buffer = *Frame::currentFrame->currentBuffer;

	switch (message)
	{
	case WM_CLOSE:
	{
		window->isOpen = false;
	} return 0;

	case WM_DESTROY:
	{
		window->isOpen = false;
		PostQuitMessage(0);
	} return 0;

	case WM_SIZE:
	{
		window->resize(LOWORD(lParam), HIWORD(lParam));
	} return 0;

	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			KeyMap::clearModifiers();
		}
	} return 0;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		// NOTE(fkp): This is just for debugging
		case '1':
		{
			if (IS_KEY_PRESSED(VK_CONTROL))
			{
				if (window->renderer->currentFont->name == "arial")
				{
					window->setFont(Font::get("consolas-24"));
				}
				else if (window->renderer->currentFont->name == "consolas-24")
				{
					window->setFont(Font::get("consolas-16"));
				}
				else if (window->renderer->currentFont->name == "consolas-16")
				{
					window->setFont(Font::get("arial"));
				}

				for (Frame* frame : window->frames)
				{
					frame->getNumberOfLines(window->renderer->currentFont);
				}
			}
		} break;

		case VK_TAB:
		{
			Frame::currentFrame->insertChar('\t');
		} break;
		
		case VK_RETURN:
		{
			if (buffer.type == BufferType::MiniBuffer)
			{
				// Gets rid of the 'Execute: '
				std::string commandText = buffer.data[0].substr(buffer.data[0].find(' '));

				while (commandText.size() > 0 && commandText[0] == ' ')
				{
					commandText.erase(0, 1);
				}

				Commands::executeCommand(*window, commandText, false);
			}
			else
			{
				Frame::currentFrame->newLine();
			}
		} break;
		}
		
		KeyMap::registerKeyPress(*window, KeyMap::convertWin32CodeToKey(wParam));
	} return 0;

	case WM_SYSKEYUP:
	case WM_KEYUP:
	{
		KeyMap::registerKeyRelease(KeyMap::convertWin32CodeToKey(wParam));
	} return 0;

	case WM_SYSCHAR:
	{
		// By default plays a beep, which we don't want.
		// I don't think it does anything else.
	} return 0;
	
	case WM_CHAR:
	{
		if (!IS_KEY_PRESSED(VK_CONTROL) &&
			wParam >= 32 && wParam < 127)
		{
			Frame::currentFrame->insertChar((char) wParam);
		}

		// Hides the cursor
		POINT mousePos;
		GetCursorPos(&mousePos);
		ScreenToClient(windowHandle, &mousePos);
		
		if (mousePos.x >= 0 && mousePos.y >= 0)
		{
			while (ShowCursor(false) >= 0);
		}
	} return 0;

	case WM_MOUSEMOVE:
	{
		// Shows the cursor
		while (ShowCursor(true) <= 0);
	} return 0;

	case WM_MOUSEWHEEL:
	{
		for (Frame* frame : window->frames)
		{
			// Only want frames with buffers, not grouping ones
			if (!frame->currentBuffer)
			{
				continue;
			}

			int realFrameX;			
			unsigned int realFrameWidth;
			int frameY;
			unsigned int frameHeight;
			frame->getRect(window->renderer->currentFont, &realFrameX, &realFrameWidth, nullptr, &frameY, nullptr, &frameHeight);

			short mouseX = GET_X_LPARAM(lParam);
			short mouseY = GET_Y_LPARAM(lParam);

			if (mouseX >= realFrameX && mouseX <= realFrameX + realFrameWidth &&
				mouseY >= frameY && mouseY <= frameY + frameHeight)
			{
				int speed = 3;

				if (GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL)
				{
					speed = 8;
				}
				
				frame->moveView((GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA) * -speed, false);
				break;
			}
		}
	} return 0;
	
	default:
	{
	} return DefWindowProc(windowHandle, message, wParam, lParam);
	}
}

void APIENTRY Window::debugLogCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
#if defined(PANDEDIT_DEBUG_WINDOW)
	// NOTE(fkp): This suppresses useless warnings

	// if (id == 131218 /* fragment shader recompiled */)
	// {
	//	return;
	// }

	// TODO(fkp): Figure out if this affects other messages
	if (source == GL_DEBUG_SOURCE_API &&
		type == GL_DEBUG_TYPE_PERFORMANCE &&
		severity == GL_DEBUG_SEVERITY_MEDIUM &&
		id == 2)
	{
		return;
	}
	
	std::string outputMessage = "[Log] ";

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               outputMessage += "Error: ";					break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: outputMessage += "Deprecated Behaviour: ";	break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  outputMessage += "Undefined Behaviour: ";	break;
	case GL_DEBUG_TYPE_PORTABILITY:         outputMessage += "Portability: ";			break;
	case GL_DEBUG_TYPE_PERFORMANCE:         outputMessage += "Performance: ";			break;
	case GL_DEBUG_TYPE_MARKER:              outputMessage += "Marker: ";				break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          outputMessage += "Push Group: ";			break;
	case GL_DEBUG_TYPE_POP_GROUP:           outputMessage += "Pop Group: ";				break;
	case GL_DEBUG_TYPE_OTHER:               outputMessage += "Other: ";					break;
	}

	outputMessage += message;
	outputMessage += "(ID: ";
	outputMessage += std::to_string(id);
	outputMessage += ")(Source: ";

	switch (source)
    {
	case GL_DEBUG_SOURCE_API:             outputMessage += "API";				break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   outputMessage += "Window System";		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: outputMessage += "Shader Compiler";	break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     outputMessage += "Third Party";		break;
	case GL_DEBUG_SOURCE_APPLICATION:     outputMessage += "Application";		break;
	case GL_DEBUG_SOURCE_OTHER:           outputMessage += "Other";				break;
    }

	outputMessage += ")(Severity: ";

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:			outputMessage += "High";		break;
	case GL_DEBUG_SEVERITY_MEDIUM:			outputMessage += "Medium";		break;
	case GL_DEBUG_SEVERITY_LOW:				outputMessage += "Low";			break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:	outputMessage += "Notice";		break;
	}

	outputMessage += ")";
	printf("%s\n", outputMessage.c_str());
#endif
}

void Window::draw()
{
	for (Frame* frame : frames)
	{
		renderer->drawFrame(*frame);
	}

	// Pop-ups should be over the top of all frames
	for (Frame* frame : frames)
	{
		renderer->drawFramePopups(*frame);
	}
}

void Window::resize(unsigned int newWidth, unsigned int newHeight)
{
	width = newWidth;
	height = newHeight;
	
	for (Frame* frame : frames)
	{
		frame->updateWindowSize(width, height - renderer->currentFont->size, renderer->currentFont);
	}

	Matrix4 projection = Matrix4::ortho(0, width, 0, height, -1, 1);
	renderer->updateShaderUniforms(projection, width, height);
	glViewport(0, 0, width, height);
}

void Window::setFont(Font* font)
{
	if (!font) return;

	renderer->currentFont = font;
	resize(width, height);
}

void Window::parseArguments(std::vector<std::string>&& args)
{
	std::string relativeExePath = getPathOnly(args[0]);
	currentProject.currentWorkingDirectory = std::filesystem::absolute(".").generic_string() + '/';
}

void Window::moveToNextFrame(bool moveNext)
{
	// This shouldn't happen, but it's there just in case
	if (Frame::currentFrame == Frame::minibufferFrame)
	{
		Frame::get("mainFrame")->makeActive();
		return;
	}

	// TODO(fkp): This can be more efficient by storing an index
	for (int i = 0; i < frames.size(); i++)
	{
		if (frames[i] == Frame::currentFrame)
		{
			// This will wrap to the current frame if necessary
			do
			{
				if (moveNext)
				{
					i += 1;
					i %= frames.size();
				}
				else
				{
					if (i == 0)
					{
						i = frames.size();
					}

					i -= 1;
				}
			} while ((frames[i]->childOne != nullptr || frames[i]->childTwo != nullptr) ||
					 frames[i]->currentBuffer->type == BufferType::MiniBuffer);

			frames[i]->makeActive();
			return;
		}
	}

	printf("Error: Window is not keeping track of current frame.\n");
}

// NOTE(fkp): Volatile! Ensure this is synced with loadProject()
void Window::saveProject(const std::string& projectName)
{
	std::ofstream file(projectName, std::ios::trunc);

	if (!file)
	{
		printf("Error: Unable to open file '%s' to save project.\n", projectName.c_str());
		return;
	}

	for (Frame* frame : frames)
	{
		file << "frame";
		
		if (frame->childOne || frame->childTwo)
		{
			file << ",group";
		}
		else
		{
			file << ",content";
		}

		// The status of the frame
		if (frame == Frame::currentFrame)
		{
			file << ",current";
		}
		else if (frame == Frame::previousFrame)
		{
			file << ",previous";
		}
		else
		{
			file << ",none";
		}
		
		file << "," << (frame->name == "" ? " " : frame->name);
		
		file << "," << frame->pcDimensions.x;
		file << "," << frame->pcDimensions.y;
		file << "," << frame->pcDimensions.width;
		file << "," << frame->pcDimensions.height;
		
		// Parent and children
		int parentIndex = -1;
		int childOneIndex = -1;
		int childTwoIndex = -1;

		for (int i = 0; i < frames.size(); i++)
		{
			if (frames[i] == frame->parent)
			{
				parentIndex = i;
			}
			else if (frames[i] == frame->childOne)
			{
				childOneIndex = i;
			}
			else if (frames[i] == frame->childTwo)
			{
				childTwoIndex = i;
			}
		}

		file << "," << parentIndex;
		file << "," << childOneIndex;
		file << "," << childTwoIndex;
		
		if (frame->childOne || frame->childTwo)
		{
			// This frame is only for grouping
		}
		else
		{
			// TODO(fkp): Saving undo information?
			file << "," << (int) frame->currentBuffer->type;
			file << "," << (frame->currentBuffer->name == "" ? " " : frame->currentBuffer->name);
			file << "," << (frame->currentBuffer->path == "" ? " " : frame->currentBuffer->path);
		
			file << "," << frame->point.line;
			file << "," << frame->point.col;
			file << "," << frame->mark.line;
			file << "," << frame->mark.col;
		
			file << "," << frame->targetTopLine;
			file << "," << frame->overwriteMode;
		}
		
		file << "\n";
	}

	// TODO(fkp): Save common properties like font
}

// NOTE(fkp): Volatile! Ensure this is synced with saveProject()
void Window::loadProject(const std::string& projectName)
{
#define READ_STRING_UNTIL_COMMA(name)			\
	std::getline(line, name, ',');

#define READ_FLOAT_UNTIL_COMMA(name, nameStr)	\
	std::string nameStr;						\
	READ_STRING_UNTIL_COMMA(nameStr);			\
	name = std::stof(nameStr);
	
#define READ_UINT_UNTIL_COMMA(name, nameStr)	\
	std::string nameStr;						\
	READ_STRING_UNTIL_COMMA(nameStr);			\
	name = std::stoul(nameStr);

#define READ_INT_UNTIL_COMMA(name, nameStr)		\
	std::string nameStr;						\
	READ_STRING_UNTIL_COMMA(nameStr);			\
	name = std::stoi(nameStr);

#define READ_BOOL_UNTIL_COMMA(name, nameStr)	\
	std::string nameStr;						\
	READ_STRING_UNTIL_COMMA(nameStr);			\
	name = (bool) std::stoi(nameStr);

	std::ifstream file(projectName);

	if (!file)
	{
		printf("Error: Unable to open project file '%s'.", projectName.c_str());
		return;
	}

	// Delete all the old frames
	for (Frame* frame : frames)
	{
		delete frame;
		frame = nullptr;
	}

	frames.clear();

	// Each line represents a complete *thing*
	std::string lineStr;
	
	while (std::getline(file, lineStr))
	{
		std::stringstream line { lineStr };
		std::string lineType;
		READ_STRING_UNTIL_COMMA(lineType);

		if (lineType == "frame")
		{
			std::string contentOrGroup;
			std::string frameStatus;
			std::string frameName;
			READ_STRING_UNTIL_COMMA(contentOrGroup);
			READ_STRING_UNTIL_COMMA(frameStatus);
			READ_STRING_UNTIL_COMMA(frameName);

			Vector4f frameDimensions;
			READ_FLOAT_UNTIL_COMMA(frameDimensions.x, dimensionsXStr);
			READ_FLOAT_UNTIL_COMMA(frameDimensions.y, dimensionsYStr);
			READ_FLOAT_UNTIL_COMMA(frameDimensions.width, dimensionsWidthStr);
			READ_FLOAT_UNTIL_COMMA(frameDimensions.height, dimensionsHeightStr);

			// Creates the new frame instance
			Frame* frame = new Frame(frameName, frameDimensions, width, height - renderer->currentFont->size);
			frames.push_back(frame);

			if (frameStatus == "current")
			{
				Frame::currentFrame = frame;
			}
			else if (frameStatus == "previous")
			{
				Frame::previousFrame = frame;
			}
			
			int parentIndex;
			int childOneIndex;
			int childTwoIndex;
			READ_INT_UNTIL_COMMA(parentIndex, parentIndexStr);
			READ_INT_UNTIL_COMMA(childOneIndex, childOneIndexStr);
			READ_INT_UNTIL_COMMA(childTwoIndex, childTwoIndexStr);

			// Removed braces for compactness
			if (parentIndex == -1)
				frame->parent = (Frame*) SIZE_MAX;
			else
				frame->parent = (Frame*) (std::size_t) parentIndex;

			if (childOneIndex == -1)
				frame->childOne = (Frame*) SIZE_MAX;
			else
				frame->childOne = (Frame*) (std::size_t) childOneIndex;

			if (childTwoIndex == -1)
				frame->childTwo = (Frame*) SIZE_MAX;
			else
				frame->childTwo = (Frame*) (std::size_t) childTwoIndex;

			if (contentOrGroup == "group")
			{
			}
			else if (contentOrGroup == "content")
			{
				int frameBufferTypeInt;
				std::string frameBufferName;
				std::string frameBufferPath;
				READ_INT_UNTIL_COMMA(frameBufferTypeInt, frameBufferTypeStr);
				READ_STRING_UNTIL_COMMA(frameBufferName);
				READ_STRING_UNTIL_COMMA(frameBufferPath);

				frame->currentBuffer = new Buffer(BufferType(frameBufferTypeInt), frameBufferName, frameBufferPath);

				if (frame->currentBuffer->type == BufferType::MiniBuffer)
				{
					Frame::minibufferFrame = frame;
				}
		
				READ_UINT_UNTIL_COMMA(frame->point.line, pointLineStr);
				READ_UINT_UNTIL_COMMA(frame->point.col, pointColStr);
				READ_UINT_UNTIL_COMMA(frame->mark.line, markLineStr);
				READ_UINT_UNTIL_COMMA(frame->mark.col, markColStr);
		
				READ_INT_UNTIL_COMMA(frame->targetTopLine, targetTopLineStr);
				frame->currentTopLine = frame->targetTopLine;
				frame->getNumberOfLines(renderer->currentFont);

				READ_BOOL_UNTIL_COMMA(frame->overwriteMode, overwriteModeStr);
			}
			else
			{
				printf("Error: Frame was neither content nor group.\n");
			}
		}
	}
	
	// Sorts out the parents and children
	// NOTE(fkp): The parent, childOne, and childTwo members of the
	// frame are used as indices into the vector of all frames. It is
	// set to SIZE_MAX if there is no parent/child.
	for (Frame* frame : frames)
	{
		if (frame->parent == (Frame*) SIZE_MAX)
		{
			frame->parent = nullptr;
		}
		else
		{
			frame->parent = frames[(std::size_t) frame->parent];
		}

		if (frame->childOne == (Frame*) SIZE_MAX)
		{
			frame->childOne = nullptr;
		}
		else
		{
			frame->childOne = frames[(std::size_t) frame->childOne];
		}
		
		if (frame->childTwo == (Frame*) SIZE_MAX)
		{
			frame->childTwo = nullptr;
		}
		else
		{
			frame->childTwo = frames[(std::size_t) frame->childTwo];
		}
	}
}


//
// Internal window creation stuff
//

bool Window::registerWindowClass()
{
	WNDCLASS windowClass {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = Window::eventCallback;
	windowClass.hInstance = GetModuleHandle(0);
	windowClass.lpszClassName = title;

	if (!RegisterClass(&windowClass))
	{
		printf("Error: Failed to register window class.\n");
		return false;
	}

	return true;
}

bool Window::createDummyWindow()
{
	windowHandle = CreateWindowEx(0, title, title,
								  windowStyles,
								  0, 0, 1, 1,
								  0, 0, GetModuleHandle(0), 0);

	if (!windowHandle)
	{
		printf("Error: Failed to create dummy window.\n");
		return false;
	}

	deviceContext = GetDC(windowHandle);

	if (!deviceContext)
	{
		printf("Error: Failed to create device context for dummy window.\n");
		return false;
	}

	// Tells Windows the format of the dummy display
	PIXELFORMATDESCRIPTOR dummyPFD {};
	dummyPFD.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	dummyPFD.nVersion = 1;
	dummyPFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	dummyPFD.iPixelType = PFD_TYPE_RGBA;
	dummyPFD.cColorBits = 24;
	dummyPFD.cDepthBits = 16;
	dummyPFD.iLayerType = PFD_MAIN_PLANE;

	// Sets the pixel format
	unsigned int dummyPixelFormat = ChoosePixelFormat(deviceContext, &dummyPFD);

	if (!dummyPixelFormat)
	{
		printf("Error: Failed to find a suitable pixel format for dummy window.\n");
		return false;
	}

	if (!SetPixelFormat(deviceContext, dummyPixelFormat, &dummyPFD))
	{
		printf("Error: Failed to set pixel format for dummy window.\n");
		return false;
	}

	// Creates the dummy rendering context
	renderingContext = wglCreateContext(deviceContext);

	if (!renderingContext)
	{
		printf("Error: Failed to create rendering context for dummy window.\n");
		return false;
	}

	if (!wglMakeCurrent(deviceContext, renderingContext))
	{
		printf("Error: Failed to make dummy contexts current.\n");
		return false;
	}

	return true;
}

bool Window::loadWGLExtensions()
{
	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) wglGetProcAddress("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");

	if (!(wglChoosePixelFormatARB && wglCreateContextAttribsARB && wglSwapIntervalEXT))
	{
		printf("Error: Failed to load WGL extensions.\n");
		return false;
	}

	return true;
}

bool Window::destroyWindowComponents()
{
	if (!wglMakeCurrent(0, 0))
	{
		printf("Error: Failed to release rendering context.\n");
		return false;
	}

	if (!wglDeleteContext(renderingContext))
	{
		printf("Error: Failed to delete rendering context.\n");
		return false;
	}

	if (!ReleaseDC(windowHandle, deviceContext))
	{
		printf("Error: Failed to release device context.\n");
		return false;
	}

	if (!DestroyWindow(windowHandle))
	{
		printf("Error: Failed to destroy window.\n");
		return false;
	}

	return true;
}

bool Window::createActualWindow()
{
	// Gets window rect
	RECT windowRect;
	windowRect.left = 0;
	windowRect.top = 0;
	windowRect.right = width;
	windowRect.bottom = height;
	AdjustWindowRect(&windowRect, windowStyles, false);

	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	// Creates the actual window
	windowHandle = CreateWindowEx(0, title, title,
								  windowStyles,
								  CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
								  0, 0, GetModuleHandle(0), 0);

	if (!windowHandle)
	{
		printf("Error: Failed to create window.\n");
		return false;
	}

	// Gets the new device context
	deviceContext = GetDC(windowHandle);

	if (!deviceContext)
	{
		printf("Error: Failed to create device context.\n");
		return false;
	}

	// Tells Windows the format of the display
	const int pixelFormatAttribs[] = {
		WGL_DRAW_TO_WINDOW_ARB,		GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,		GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
		WGL_PIXEL_TYPE_ARB,			WGL_TYPE_RGBA_ARB,
		WGL_ACCELERATION_ARB,		WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB,			24,
		WGL_ALPHA_BITS_ARB,			8,
		WGL_DEPTH_BITS_ARB,			16,
		WGL_STENCIL_BITS_ARB,		8,
		WGL_SAMPLE_BUFFERS_ARB,		GL_TRUE,
		WGL_SAMPLES_ARB,			4,
		0
	};

	// Sets the pixel format
	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	int pixelFormat;
	unsigned int numberOfFormats;
	bool result = wglChoosePixelFormatARB(deviceContext, pixelFormatAttribs, 0, 1, &pixelFormat, &numberOfFormats);

	if (!result || numberOfFormats == 0)
	{
		printf("Error: Failed to choose a pixel format.\n");
		return false;
	}

	if (!DescribePixelFormat(deviceContext, pixelFormat, sizeof(pixelFormatDescriptor), &pixelFormatDescriptor))
	{
		printf("Error: Failed to describe a proper pixel format.\n");
		return false;
	}

	if (!SetPixelFormat(deviceContext, pixelFormat, &pixelFormatDescriptor))
	{
		printf("Error: Failed to set a proper pixel format.\n");
		return false;
	}

	// Creates a rendering context
	int renderingContextAttribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 4,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#if defined(PANDEDIT_DEBUG_WINDOW)
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
		0
	};

	renderingContext = wglCreateContextAttribsARB(deviceContext, 0, renderingContextAttribs);

	if (!renderingContext)
	{
		printf("Error: Failed to create an OpenGL rendering context.\n");
		return false;
	}

	// Makes the actual contexts current
	if (!wglMakeCurrent(deviceContext, renderingContext))
	{
		printf("Error: Failed to make rendering context current.\n");
		return false;
	}

#if defined(PANDEDIT_VSYNC)
	wglSwapIntervalEXT(1);
#else
	wglSwapIntervalEXT(0);
#endif

	// Loads all OpenGL function pointers
	gladLoadGL();

	glViewport(0, 0, width, height);

	// Shows the window
	ShowWindow(windowHandle, SW_SHOWNORMAL);

	return true;
}

void Window::registerDebugContextCallback()
{
#if defined (PANDEDIT_DEBUG_WINDOW)
	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		
		glDebugMessageCallback(debugLogCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
	else
	{
		printf("Warning: Debug context requested but not available.\n");
	}
#endif
}
