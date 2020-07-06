//  ===== Date Created: 04 July, 2020 ===== 

#if !defined(PROJECT_HPP)
#define PROJECT_HPP

#include <string>
#include <future>
#include <thread>

#include "buffer.hpp"

class Window;

class Project
{
public:
	std::string currentPath = "";
	std::string currentWorkingDirectory = "";
	
	std::string compileCommand = "";
	std::future<bool> compileFuture;
	std::thread compileThread;

public:
	void saveToFile(const std::string& path, const Window& window);
	void loadFromFile(const std::string& path, Window& window);
	
	bool isCompileRunning();
	bool executeCompileCommand(Buffer* compileBuffer);
};

#endif
