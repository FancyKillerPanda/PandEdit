//  ===== Date Created: 17 April, 2020 ===== 

#include "commands.hpp"
#include "frame.hpp"

// TODO(fkp): Write to minibuffer function

#define DEFINE_COMMAND(name) bool name(const std::string& text)

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

std::unordered_map<std::string, bool (*)(const std::string& text)> Commands::commandsMap = {
	{ "echo", echo_Command },
};

void Commands::executeCommand(const std::string& commandText)
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
		if (result->second(argumentsText))
		{
			// Exits the minibuffer
			Frame::minibufferFrame->currentBuffer->col = 0; // Just to be safe
			Frame::previousFrame->makeActive();
		}
	}
	else
	{
		Frame::minibufferFrame->currentBuffer->data[0] = "Error: Unknown command";
		Frame::minibufferFrame->currentBuffer->col = Frame::minibufferFrame->currentBuffer->data[0].size();
		Frame::previousFrame->makeActive();		
	}
}
