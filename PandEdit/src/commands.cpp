//  ===== Date Created: 17 April, 2020 ===== 

#include "commands.hpp"
#include "frame.hpp"
#include "window.hpp"

void writeToMinibuffer(std::string message)
{
	Frame::minibufferFrame->currentBuffer->data[0] = message;
	Frame::minibufferFrame->point.col = Frame::minibufferFrame->currentBuffer->data[0].size();
}

void exitMinibuffer()
{
	if (Frame::currentFrame->currentBuffer->type == BufferType::MiniBuffer)
	{
		Frame::minibufferFrame->point.col = 0; // Just to be safe
		Frame::previousFrame->makeActive();
	}
}

void exitMinibuffer(std::string message)
{
	writeToMinibuffer(message);
	exitMinibuffer();
}

#include "commands_definitions.inl"
#define COMMAND(name) { #name, name }

std::unordered_map<std::string, COMMAND_FUNC_SIG()> Commands::essentialCommandsMap = {
	COMMAND(quit),
	COMMAND(minibufferQuit),
	
	COMMAND(backspaceChar),
	COMMAND(deleteChar),
	COMMAND(backspaceWord),
	COMMAND(deleteWord),
	
	COMMAND(movePointLeftChar),
	COMMAND(movePointRightChar),
	COMMAND(movePointLeftWord),
	COMMAND(movePointRightWord),
	COMMAND(movePointLineUp),
	COMMAND(movePointLineDown),
	COMMAND(movePointHome),
	COMMAND(movePointEnd),
	COMMAND(setMark),
	COMMAND(swapPointAndMark),
	
	COMMAND(pageUp),
	COMMAND(pageDown),
	COMMAND(centerPoint),

	COMMAND(copyRegion),
	COMMAND(paste),
	COMMAND(pastePop),
	COMMAND(deleteRegion),
	COMMAND(undo),
	COMMAND(redo),
};

std::unordered_map<std::string, COMMAND_FUNC_SIG()> Commands::nonEssentialCommandsMap = {
	COMMAND(echo),
	COMMAND(minibufferEnter),

	COMMAND(frameSplitVertically),
	COMMAND(frameSplitHorizontally),
	COMMAND(frameMoveNext),
	COMMAND(frameMovePrevious),
	
	COMMAND(switchToBuffer),
	COMMAND(findFile),
	COMMAND(saveCurrentBuffer),
};

#undef COMMAND

void Commands::executeCommand(Window& window, const std::string& commandText)
{	
	std::string commandName = commandText.substr(0, commandText.find(' '));
	std::string argumentsText = "";

	if (commandText.find(' ') < commandText.size())
	{
		argumentsText = commandText.substr(commandText.find(' '));
	}

	// Strips the leading spaces
	if (argumentsText.size() > 0 && argumentsText[0] == ' ')
	{
		argumentsText.erase(0, 1);
	}

	auto result = essentialCommandsMap.find(commandName);

	if (result != essentialCommandsMap.end())
	{
		if (result->second(window, argumentsText))
		{
			exitMinibuffer();
		}
		
		lastCommand = commandName;
	}
	else
	{
		if (currentCommand)
		{
			currentCommand(window, commandText);
		}
		else
		{
			result = nonEssentialCommandsMap.find(commandName);

			if (result != nonEssentialCommandsMap.end())
			{
				if (result->second(window, argumentsText))
				{
					exitMinibuffer();
				}
				
				lastCommand = commandName;
			}
			else
			{
				exitMinibuffer("Error: Unknown command");
			}
		}
	}
}
