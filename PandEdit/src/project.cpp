//  ===== Date Created: 04 July, 2020 ===== 

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <thread>

#include "project.hpp"

bool Project::isCompileRunning()
{
	using namespace std::chrono_literals;

	// Try catch is needed as this method could be called before a compilation
	// which throws a no_state error
	try
	{
		std::future_status compileStatus = compileFuture.wait_for(0ms);
		return compileStatus != std::future_status::ready;
	}
	catch (const std::future_error& error)
	{
		return false;
	}
}

// This method will run asynchronously after a compilation has begun
void waitForCompilationToFinish(PROCESS_INFORMATION processInformation, std::promise<bool>& compilePromise)
{
	// Waits for an event or the process to finish
	MSG message;
	DWORD reason = WAIT_TIMEOUT;

	while (reason != WAIT_OBJECT_0)
	{
		reason = MsgWaitForMultipleObjects(1, &processInformation.hProcess, false, 0, QS_ALLINPUT);

		if (reason == WAIT_OBJECT_0)
		{
			printf("Info: Finished compilation.\n");
		}
		else if (reason == WAIT_OBJECT_0 + 1)
		{
			// A window message is available
			while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
	}

	// Cleanup of the child process
	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);

	compilePromise.set_value(true);
}

bool Project::executeCompileCommand()
{
	if (isCompileRunning())
	{
		return false;
	}
	
	STARTUPINFO startupInformation {};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation {};

	// Makes the command a modifiable char* because Windows wants that
	// for some reason
	char* compileCommandCStr = new char[compileCommand.size() + 1];
	std::copy(compileCommand.begin(), compileCommand.end(), compileCommandCStr);
	compileCommandCStr[compileCommand.size()] = '\0';
	
	if (CreateProcess(NULL, compileCommandCStr,
					  NULL, NULL, false, 0, NULL, NULL,
					  &startupInformation, &processInformation))
	{
		std::promise<bool> compilePromise;
		compileFuture = compilePromise.get_future();
		
		compileThread = std::thread { waitForCompilationToFinish, processInformation, std::ref(compilePromise) };
		compileThread.detach();
	}
	else
	{
		printf("Error: could not create process to compile.\n");
		return false;
	}

	delete[] compileCommandCStr;
	// TODO(fkp): Delete the temporary __compile__.pe file

	return true;
}
