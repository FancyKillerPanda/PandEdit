//  ===== Date Created: 04 July, 2020 ===== 

#if !defined(PROJECT_HPP)
#define PROJECT_HPP

#include <string>

// TODO(fkp): Move project save and load to be methods here
class Project
{
public:
	std::string currentWorkingDirectory;
	std::string compileCommand = "cmd.exe /C build.bat > __compile__.pe";

public:
	bool executeCompileCommand();
};

#endif
