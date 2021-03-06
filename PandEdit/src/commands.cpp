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
		Frame::minibufferFrame->popupLines.clear();
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

// TODO(fkp): Try to remember the criteria for what is and isn't
// essential and see if everything still fits correctly.
std::unordered_map<std::string, COMMAND_FUNC_SIG()> Commands::essentialCommandsMap = {
	COMMAND(quit),
	COMMAND(minibufferQuit),
	
	COMMAND(backspaceChar),
	COMMAND(backspaceCharExtra),
	COMMAND(deleteChar),
	COMMAND(backspaceWord),
	COMMAND(deleteWord),
	COMMAND(deleteRestOfLine),
	
	COMMAND(movePointLeftChar),
	COMMAND(movePointRightChar),
	COMMAND(movePointLeftWord),
	COMMAND(movePointRightWord),
	COMMAND(movePointLineUp),
	COMMAND(movePointLineDown),
	COMMAND(movePointHome),
	COMMAND(movePointEnd),
	COMMAND(movePointToBufferStart),
	COMMAND(movePointToBufferEnd),
	
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
	
	COMMAND(completeSuggestion),
	COMMAND(nextSuggestion),
	COMMAND(previousSuggestion),
};

std::unordered_map<std::string, COMMAND_FUNC_SIG()> Commands::nonEssentialCommandsMap = {
	COMMAND(echo),
	COMMAND(minibufferEnter),
	COMMAND(toggleOverwriteMode),

	COMMAND(frameSplitVertically),
	COMMAND(frameSplitHorizontally),
	COMMAND(frameDestroy),
	COMMAND(frameMoveNext),
	COMMAND(frameMovePrevious),
	
	COMMAND(switchToBuffer),
	COMMAND(destroyBuffer),
	COMMAND(findFile),
	COMMAND(saveCurrentBuffer),
	COMMAND(saveAllBuffers),
	COMMAND(revertBuffer),

	{ "lexBufferAsC++", lexBufferAsCpp },

	COMMAND(saveProject),
	COMMAND(loadProject),
	COMMAND(compile),
};

#undef COMMAND

void Commands::executeCommand(Window& window, const std::string& commandText, bool shortcut)
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
			// Should only happen if this was typed in, not through a keyboard shortcut.
			if (!shortcut)
			{
				currentCommand(window, commandText);
			}
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
