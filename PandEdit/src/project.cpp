//  ===== Date Created: 04 July, 2020 ===== 

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <thread>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "project.hpp"
#include "window.hpp"
#include "frame.hpp"
#include "renderer.hpp"
#include "font.hpp"

// NOTE(fkp): Volatile! Ensure this is synced with loadFromFile()
void Project::saveToFile(const std::string& path, const Window& window)
{
	currentPath = path;
	std::ofstream file(path, std::ios::trunc);

	if (!file)
	{
		printf("Error: Unable to open file '%s' to save project.\n", path.c_str());
		return;
	}

	for (const Frame* frame : window.frames)
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

		for (int i = 0; i < window.frames.size(); i++)
		{
			if (window.frames[i] == frame->parent)
			{
				parentIndex = i;
			}
			else if (window.frames[i] == frame->childOne)
			{
				childOneIndex = i;
			}
			else if (window.frames[i] == frame->childTwo)
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

	file << "compileCommand," << compileCommand << "\n";
}

// NOTE(fkp): Volatile! Ensure this is synced with saveToFile()
void Project::loadFromFile(const std::string& path, Window& window)
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

	currentPath = path;
	std::ifstream file(path);

	if (!file)
	{
		printf("Error: Unable to open project file '%s'.", path.c_str());
		return;
	}

	// Delete all the old frames
	for (Frame* frame : window.frames)
	{
		delete frame;
		frame = nullptr;
	}

	window.frames.clear();

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
			Frame* frame = new Frame(frameName, frameDimensions, window.width, window.height - window.renderer->currentFont->size);
			window.frames.push_back(frame);

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
				frame->getNumberOfLines(window.renderer->currentFont);

				READ_BOOL_UNTIL_COMMA(frame->overwriteMode, overwriteModeStr);
			}
			else
			{
				printf("Error: Frame was neither content nor group.\n");
			}
		}
		else if (lineType == "compileCommand")
		{
			std::string command = lineStr.substr(lineStr.find_first_of(",") + 1);

			if (command == "")
			{
				printf("Warning: No compile command specified in project file.\n");
			}
			else
			{
				compileCommand = "cmd.exe /C " + command;
			}
		}
	}
	
	// Sorts out the parents and children
	// NOTE(fkp): The parent, childOne, and childTwo members of the
	// frame are used as indices into the vector of all frames. It is
	// set to SIZE_MAX if there is no parent/child.
	for (Frame* frame : window.frames)
	{
		if (frame->parent == (Frame*) SIZE_MAX)
		{
			frame->parent = nullptr;
		}
		else
		{
			frame->parent = window.frames[(std::size_t) frame->parent];
		}

		if (frame->childOne == (Frame*) SIZE_MAX)
		{
			frame->childOne = nullptr;
		}
		else
		{
			frame->childOne = window.frames[(std::size_t) frame->childOne];
		}
		
		if (frame->childTwo == (Frame*) SIZE_MAX)
		{
			frame->childTwo = nullptr;
		}
		else
		{
			frame->childTwo = window.frames[(std::size_t) frame->childTwo];
		}
	}
}

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
void waitForCompilationToFinish(HANDLE childStdoutRead)
{
	// Reads from the child process's STDOUT
	const unsigned int BUFFER_SIZE = 1023;
	char buffer[BUFFER_SIZE + 1];
	DWORD numberOfBytesRead = 0;

	while (true)
	{
		bool success = ReadFile(childStdoutRead, buffer, BUFFER_SIZE, &numberOfBytesRead, nullptr);
		if (!success || numberOfBytesRead == 0) break;

		// Don't know if we need to add an explicit null-termination
		// character, I couldn't find it in the documentation and
		// don't want to take any chances.
		buffer[numberOfBytesRead] = 0;
		printf("Read: %s", buffer);
	}

	// Closes the one remaining handle
	CloseHandle(childStdoutRead);

	printf("Info: Finished compilation.\n");
}

bool Project::executeCompileCommand()
{
	if (isCompileRunning())
	{
		printf("Error: Failed to start new compilation, one already running.\n");
		return false;
	}

	// Makes the command a modifiable char* because Windows wants that
	// for some reason
	char* compileCommandCStr = new char[compileCommand.size() + 1];
	std::copy(compileCommand.begin(), compileCommand.end(), compileCommandCStr);
	compileCommandCStr[compileCommand.size()] = '\0';
	
	//
	// NOTE(fkp): Pipe setup
	//
	
	HANDLE childStdOutRead = nullptr;
	HANDLE childStdOutWrite = nullptr;
	HANDLE childStdInRead = nullptr;
	HANDLE childStdInWrite = nullptr;
	
	SECURITY_ATTRIBUTES securityAttributes {};
	securityAttributes.nLength = sizeof(securityAttributes);
	securityAttributes.bInheritHandle = true;
	securityAttributes.lpSecurityDescriptor = nullptr;

 	// Create a pipe for the child process's STDOUT
	if (!CreatePipe(&childStdOutRead, &childStdOutWrite, &securityAttributes, 0))
	{
		printf("Error: Failed to create child process's STDOUT handle.\n");
		return false;
	}

	// Ensure child's STDOUT read handle is not inherited
	if (!SetHandleInformation(childStdOutRead, HANDLE_FLAG_INHERIT, 0))
	{
		printf("Error: Failed to ensure child process's STDOUT read handle is not inherited.\n");
		return false;
	}

 	// Create a pipe for the child process's STDIN
	if (!CreatePipe(&childStdInRead, &childStdInWrite, &securityAttributes, 0))
	{
		printf("Error: Failed to create child process's STDIN handle.\n");
		return false;
	}

	// Ensure child's STDIN write handle is not inherited
	if (!SetHandleInformation(childStdInWrite, HANDLE_FLAG_INHERIT, 0))
	{
		printf("Error: Failed to ensure child process's STDIN write handle is not inherited.\n");
		return false;
	}

	
	//
	// NOTE(fkp): Process creation
	//

	PROCESS_INFORMATION processInformation {};
	STARTUPINFO startupInfo {};
	startupInfo.cb = sizeof(startupInfo);
	startupInfo.hStdError = childStdOutWrite;
	startupInfo.hStdOutput = childStdOutWrite;
	startupInfo.hStdInput = childStdInRead;
	startupInfo.dwFlags = STARTF_USESTDHANDLES;

	if (CreateProcess(nullptr, compileCommandCStr,
					  nullptr, nullptr, true, 0, nullptr, nullptr,
					  &startupInfo, &processInformation))
	{
		std::promise<bool> compilePromise;
		compileFuture = compilePromise.get_future();

		compileThread = std::thread { waitForCompilationToFinish, childStdOutRead };
		compileThread.detach();
		
		// Closes unneeded handles
		// NOTE(fkp): childStdOutRead will be closed in waitForCompilationToFinish()
		CloseHandle(processInformation.hProcess);
		CloseHandle(processInformation.hThread);
		CloseHandle(childStdOutWrite);
		CloseHandle(childStdInRead);
		CloseHandle(childStdInWrite);
	}
	else
	{
		printf("Error: Failed to create child process.\n");
		return false;
	}

	return true;
}
