//  ===== Date Created: 04 July, 2020 ===== 

#if !defined(PROJECT_HPP)
#define PROJECT_HPP

#include <string>
#include <future>
#include <thread>

// TODO(fkp): Move project save and load to be methods here
class Project
{
public:
	std::string currentWorkingDirectory;
	std::string compileCommand = "cmd.exe /C build.bat > __compile__.pe";
	
	std::future<bool> compileFuture;
	std::thread compileThread;

public:
	bool isCompileRunning();
	bool executeCompileCommand();
};

#endif
