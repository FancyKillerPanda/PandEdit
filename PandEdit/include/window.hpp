//  ===== Date Created: 14 April, 2020 =====

#if !defined(WINDOW_HPP)
#define WINDOW_HPP

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <unordered_map>
#include <vector>
#include <string>

#include <glad/glad.h>

#include "frame.hpp"

class Renderer;
class Font;

class Window
{
public:
	bool isOpen;

	unsigned int width;
	unsigned int height;
	const char* title;

	HWND windowHandle;
	HDC deviceContext;
	HGLRC renderingContext;

	const DWORD windowStyles = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	Renderer* renderer;
	std::vector<Frame*> frames;

	std::string currentWorkingDirectory;

private:
	inline static std::unordered_map<HWND, Window*> windowsMap;

public:
	Window(unsigned int width, unsigned int height, const char* title);
	~Window();

	static Window* get(HWND handle);

	void draw();
	void resize(unsigned int newWidth, unsigned int newHeight);
	void setFont(Font* font);
	void parseArguments(std::vector<std::string>&& args);

	// If moveNext is false, will move backwards
	void moveToNextFrame(bool moveNext = true);

	// TODO(fkp): Should we move this to a separate place
	void saveProject(const std::string& projectName);
	void loadProject(const std::string& projectName);

private:
	static LRESULT CALLBACK eventCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
	static void APIENTRY debugLogCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

	// Window creation
	bool registerWindowClass();
	bool createDummyWindow();
	bool loadWGLExtensions();
	bool destroyWindowComponents();
	bool createActualWindow();
	void registerDebugContextCallback();
};

#endif
