//  ===== Date Created: 14 April, 2020 ===== 

#if !defined(WINDOW_HPP)
#define WINDOW_HPP

#include <windows.h>
#include <unordered_map>
#include <vector>

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
	std::vector<Frame> frames;

private:
	static std::unordered_map<HWND, Window*> windowsMap;
	
public:
	Window(unsigned int width, unsigned int height, const char* title);
	~Window();
	static Window* get(HWND handle);

	void draw();
	void setFont(Font* font);
	void splitCurrentFrameVertically();

private:
	static LRESULT CALLBACK eventCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
	
	// Window creation
	bool registerWindowClass();
	bool createDummyWindow();
	bool loadWGLExtensions();
	bool destroyWindowComponents();
	bool createActualWindow();
};

#endif
