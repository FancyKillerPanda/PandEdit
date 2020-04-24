//  ===== Date Created: 17 April, 2020 ===== 

#include "commands.hpp"
#include "frame.hpp"
#include "window.hpp"

COMMAND_FUNC_SIG(Commands::currentCommand) = nullptr;

void writeToMinibuffer(std::string message)
{
	Frame::minibufferFrame->currentBuffer->data[0] = message;
	Frame::minibufferFrame->col = Frame::minibufferFrame->currentBuffer->data[0].size();
}

void exitMinibuffer()
{
	if (Frame::currentFrame->currentBuffer->type == BufferType::MiniBuffer)
	{
		Frame::minibufferFrame->col = 0; // Just to be safe
		Frame::previousFrame->makeActive();
	}
}

void exitMinibuffer(std::string message)
{
	writeToMinibuffer(message);
	exitMinibuffer();
}

#include "commands_definitions.inl"

std::unordered_map<std::string, COMMAND_FUNC_SIG()> Commands::essentialCommandsMap = {
	{ "minibufferQuit", minibufferQuit },
	
	{ "backspaceChar", backspaceChar },
	{ "deleteChar", deleteChar },
	{ "backspaceWord", backspaceWord },
	{ "deleteWord", deleteWord },
	
	{ "movePointLeftChar", movePointLeftChar },
	{ "movePointRightChar", movePointRightChar },
	{ "movePointLeftWord", movePointLeftWord },
	{ "movePointRightWord", movePointRightWord },
	{ "movePointLineUp", movePointLineUp },
	{ "movePointLineDown", movePointLineDown },
	{ "movePointHome", movePointHome },
	{ "movePointEnd", movePointEnd },
	{ "setMark", setMark },
	{ "swapPointAndMark", swapPointAndMark },
};

std::unordered_map<std::string, COMMAND_FUNC_SIG()> Commands::nonEssentialCommandsMap = {
	{ "echo", echo },
	{ "minibufferEnter", minibufferEnter },

	{ "frameSplitVertically", frameSplitVertically },
	{ "frameSplitHorizontally", frameSplitHorizontally },
	{ "frameMoveNext", frameMoveNext },
	{ "frameMovePrevious", frameMovePrevious },
	
	{ "switchToBuffer", switchToBuffer },
};

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
			}
			else
			{
				exitMinibuffer("Error: Unknown command");
			}
		}
	}
}
