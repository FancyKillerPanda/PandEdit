//  ===== Date Created: 17 April, 2020 ===== 

#include "commands.hpp"
#include "frame.hpp"
#include "window.hpp"

// TODO(fkp): Write to minibuffer function

void exitMinibuffer()
{
	if (Frame::currentFrame->currentBuffer->type == BufferType::MiniBuffer)
	{
		Frame::minibufferFrame->currentBuffer->col = 0; // Just to be safe
		Frame::previousFrame->makeActive();
	}
}

#define DEFINE_COMMAND(name) bool name(Window& window, const std::string& text)

DEFINE_COMMAND(echo_Command)
{
	if (text == "")
	{
		Frame::minibufferFrame->currentBuffer->data[0] = "Error: Nothing to echo";
	}
	else
	{
		Frame::minibufferFrame->currentBuffer->data[0] = text;
	}

	Frame::minibufferFrame->currentBuffer->col = Frame::minibufferFrame->currentBuffer->data[0].size();
	return true;
}

DEFINE_COMMAND(frameSplitVertically_Command)
{
	exitMinibuffer();
	window.splitCurrentFrameVertically();
	
	return true;
}

std::unordered_map<std::string, bool (*)(Window&, const std::string& text)> Commands::commandsMap = {
	{ "echo", echo_Command },
	{ "frameSplitVertically", frameSplitVertically_Command },
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

	auto result = commandsMap.find(commandName);

	if (result != commandsMap.end())
	{
		if (result->second(window, argumentsText))
		{
			// Exits the minibuffer
			exitMinibuffer();
		}
	}
	else
	{
		Frame::minibufferFrame->currentBuffer->data[0] = "Error: Unknown command";
		exitMinibuffer();
	}
}
