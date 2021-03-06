// NOTE(fkp): This file is not to be compiled, it will be included in
// commands.cpp

#include <fstream>

#include "file_util.hpp"
#include "renderer.hpp"
#include "lexer.hpp"

#define DEFINE_COMMAND(name) bool name(Window& window, const std::string& text)
#define FRAME Frame::currentFrame
#define BUFFER Frame::currentFrame->currentBuffer

void startReadingPath(Window& window)
{
	std::string message = "Path: " + window.currentProject.currentWorkingDirectory;
	Commands::currentlyReading = MinibufferReading::Path;
	writeToMinibuffer(message);
	FRAME->updatePopups();
}

void startReadingBufferName()
{
	Commands::currentlyReading = MinibufferReading::BufferName;
	writeToMinibuffer("Buffer: ");
	FRAME->updatePopups();
}

//
// NOTE(fkp): Minibuffer/miscellaneous commands
//

DEFINE_COMMAND(echo)
{
	if (text == "")
	{
		writeToMinibuffer("Error: Nothing to echo");
	}
	else
	{
		writeToMinibuffer(text);
	}
	return true;
}

// Forward declarations for quit
DEFINE_COMMAND(saveAllBuffers);
DEFINE_COMMAND(minibufferEnter);

DEFINE_COMMAND(quit)
{
	if (Commands::currentCommand == quit &&
		Commands::currentlyReading == MinibufferReading::Confirmation)
	{
		switch (text.back())
		{
		case 'y':
		{
			saveAllBuffers(window, text);
		} // no break

		case 'n':
		{
			window.isOpen = false;
		} return true;;

		default:
		{
			writeToMinibuffer("Save buffers? [y/n] ");
			FRAME->point.col = BUFFER->data[0].size();
		} return false;
		}
	}
	else
	{
		bool someBufferNeedsSaving = false;

		for (auto& bufferElement : Buffer::buffersMap)
		{
			if (bufferElement.second->type == BufferType::MiniBuffer) continue;
			
			if (!bufferElement.second->isReadOnly &&
				bufferElement.second->numberOfActionsSinceSave > 0)
			{
				someBufferNeedsSaving = true;
				break;
			}
		}
	
		if (someBufferNeedsSaving)
		{
			Commands::currentCommand = quit;
		
			minibufferEnter(window, text);
			writeToMinibuffer("Save buffers? [y/n] ");
			FRAME->point.col = BUFFER->data[0].size();
		
			Commands::currentlyReading = MinibufferReading::Confirmation;
			FRAME->popupLines.clear();

			return false;
		}
		else
		{
			window.isOpen = false;
			return true;
		}
	}
}

DEFINE_COMMAND(minibufferEnter)
{
	if (Frame::currentFrame != Frame::minibufferFrame)
	{
		Frame::minibufferFrame->makeActive();
		writeToMinibuffer("Execute: ");
		Commands::currentlyReading = MinibufferReading::Command;
	}
	
	return false;
}

DEFINE_COMMAND(minibufferQuit)
{
	if (Frame::currentFrame == Frame::minibufferFrame)
	{
		exitMinibuffer("Quit");
		Frame::previousFrame->makeActive();
		Commands::currentCommand = nullptr;
		Commands::currentlyReading = MinibufferReading::None;
	}
	
	return true;
}

DEFINE_COMMAND(toggleOverwriteMode)
{
	FRAME->overwriteMode = !FRAME->overwriteMode;

	if (FRAME != Frame::minibufferFrame)
	{
		std::string msg = "Overwrite mode ";
		msg += FRAME->overwriteMode ? "enabled" : "disabled";
		msg += " in the current frame.";
		
		writeToMinibuffer(msg);
	}
	
	return false;
}


//
// NOTE(fkp): Frame commands
//

DEFINE_COMMAND(frameSplitVertically)
{
	exitMinibuffer("");
	FRAME->split(true, window.renderer->currentFont);
	return true;
}

DEFINE_COMMAND(frameSplitHorizontally)
{
	exitMinibuffer("");
	FRAME->split(false, window.renderer->currentFont);
	return true;
}

DEFINE_COMMAND(frameDestroy)
{
	exitMinibuffer("");
	FRAME->destroy();
	return true;
}

DEFINE_COMMAND(frameMoveNext)
{
	exitMinibuffer("");
	window.moveToNextFrame();
	return true;	
}

DEFINE_COMMAND(frameMovePrevious)
{
	exitMinibuffer("");
	window.moveToNextFrame(false);
	return true;	
}


//
// NOTE(fkp): Text manipulation commands
//

DEFINE_COMMAND(backspaceChar)
{
	FRAME->backspaceChar(1);
	return false;
}

DEFINE_COMMAND(backspaceCharExtra)
{
	if (BUFFER->type == BufferType::MiniBuffer && Commands::currentlyReading == MinibufferReading::Path)
	{
		std::string::size_type indexSlashBefore = BUFFER->data[0].find_last_of("/\\", FRAME->point.col - 2);
		std::string::size_type indexSlashAfter = BUFFER->data[0].find_first_of("/\\", FRAME->point.col - 1);

		if (indexSlashBefore == std::string::npos)
		{
			indexSlashBefore = BUFFER->data[0].find_first_of(" ");
		}

		if (indexSlashAfter == std::string::npos)
		{
			indexSlashAfter = BUFFER->data[0].size();
		}

		FRAME->point.col = indexSlashAfter + 1;
		
		while (FRAME->point.col > indexSlashBefore + 1)
		{
			FRAME->backspaceChar(1);
		}
		
		return false;
	}
	else
	{
		// TODO(fkp): Figure out a way to forward all the arguments
		return backspaceChar(window, text);
	}
}

DEFINE_COMMAND(deleteChar)
{
	FRAME->deleteChar(1);
	return false;
}

DEFINE_COMMAND(backspaceWord)
{
	FRAME->backspaceChar(FRAME->findWordBoundaryLeft());
	return false;
}

DEFINE_COMMAND(deleteWord)
{
	FRAME->deleteChar(FRAME->findWordBoundaryRight());
	return false;
}

DEFINE_COMMAND(deleteRestOfLine)
{
	FRAME->deleteRestOfLine();
	return false;
}


//
// NOTE(fkp): Point movement commands
//

DEFINE_COMMAND(movePointLeftChar)
{
	FRAME->movePointLeft(1);
	return false;
}

DEFINE_COMMAND(movePointRightChar)
{
	FRAME->movePointRight(1);
	return false;
}

DEFINE_COMMAND(movePointLeftWord)
{
	FRAME->movePointLeft(FRAME->findWordBoundaryLeft());
	return false;
}

DEFINE_COMMAND(movePointRightWord)
{
	FRAME->movePointRight(FRAME->findWordBoundaryRight());
	return false;
}

DEFINE_COMMAND(movePointLineUp)
{
	FRAME->movePointUp();
	return false;
}

DEFINE_COMMAND(movePointLineDown)
{
	FRAME->movePointDown();
	return false;
}

DEFINE_COMMAND(movePointHome)
{
	FRAME->movePointHome();
	return false;
}

DEFINE_COMMAND(movePointEnd)
{
	FRAME->movePointEnd();
	return false;
}

DEFINE_COMMAND(movePointToBufferStart)
{
	FRAME->movePointToBufferStart();
	return false;
}

DEFINE_COMMAND(movePointToBufferEnd)
{
	FRAME->movePointToBufferEnd();
	return false;
}

DEFINE_COMMAND(setMark)
{
	FRAME->mark.line = FRAME->point.line;
	FRAME->mark.col = FRAME->point.col;

	return false;
}

DEFINE_COMMAND(swapPointAndMark)
{
	unsigned int tempLine = FRAME->point.line;
	unsigned int tempCol = FRAME->point.col;

	FRAME->point.line = FRAME->mark.line;
	FRAME->point.col = FRAME->mark.col;
	FRAME->mark.line = tempLine;
	FRAME->mark.col = tempCol;
	FRAME->point.targetCol = FRAME->point.col;

	FRAME->doCommonPointManipulationTasks();

	return false;
}

DEFINE_COMMAND(pageUp)
{
	// Minibuffer shouldn't have scrolling
	if (BUFFER->type != BufferType::MiniBuffer)
	{
		FRAME->moveView(-(FRAME->numberOfLinesInView - 3), true);
	}

	return false;
}

DEFINE_COMMAND(pageDown)
{
	// Minibuffer shouldn't have scrolling
	if (BUFFER->type != BufferType::MiniBuffer)
	{
		FRAME->moveView(FRAME->numberOfLinesInView - 3, true);
	}

	return false;
}

DEFINE_COMMAND(centerPoint)
{
	// Minibuffer shouldn't have scrolling
	if (BUFFER->type != BufferType::MiniBuffer)
	{
		FRAME->centerPoint();
	}

	return false;
}


//
// NOTE(fkp): Copy/cut/paste/undo/redo
//

DEFINE_COMMAND(copyRegion)
{
	FRAME->copyRegion();
	return false;
}

DEFINE_COMMAND(paste)
{
	FRAME->paste();
	return false;
}

DEFINE_COMMAND(pastePop)
{
	if (Commands::lastCommand == "paste" || Commands::lastCommand == "pastePop")
	{
		FRAME->pastePop();
	}

	return false;
}

DEFINE_COMMAND(deleteRegion)
{
	FRAME->deleteTextPointToMark();
	return false;
}

DEFINE_COMMAND(undo)
{
	if (!BUFFER->undo(*FRAME) && BUFFER->type != BufferType::MiniBuffer)
	{
		writeToMinibuffer("Nothing to undo.");
	}
	
	return false;
}

DEFINE_COMMAND(redo)
{
	if (!BUFFER->redo(*FRAME) && BUFFER->type != BufferType::MiniBuffer)
	{
		writeToMinibuffer("Nothing to redo.");
	}
	
	return false;	
}


//
// NOTE(fkp): Buffer commands
//

DEFINE_COMMAND(switchToBuffer)
{
	std::string bufferName = text.substr(0, text.find(' '));
	
	if (Commands::currentCommand)
	{
		if (bufferName != "")
		{
			Commands::currentCommand = nullptr;
			exitMinibuffer("");
		
			Buffer* buffer = Buffer::get(bufferName);

			if (!buffer)
			{
				buffer = new Buffer { BufferType::Text, text.substr(0, text.find(' ')), "" };
			}
		
			FRAME->switchToBuffer(buffer);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		Frame::minibufferFrame->makeActive();
		startReadingBufferName();
		Commands::currentCommand = switchToBuffer;
		
		return false;
	}
}

DEFINE_COMMAND(destroyBuffer)
{
	if (Commands::currentCommand)
	{
		std::string bufferName = text.substr(0, text.find(' '));

		if (bufferName != "")
		{
			Commands::currentCommand = nullptr;
			exitMinibuffer("");
			FRAME->destroyBuffer(Buffer::get(bufferName));
			return true;
		}
		else
		{
			return false;
		}
		
	}
	else
	{
		Frame::minibufferFrame->makeActive();
		startReadingBufferName();		
		Commands::currentCommand = destroyBuffer;

		return false;
	}
}

// TODO(fkp): This command is very similar to switchToBuffer.
DEFINE_COMMAND(findFile)
{
	if (Commands::currentCommand)
	{
		if (Commands::currentlyReading == MinibufferReading::Path)
		{
			// Checks if the file exists
			std::ifstream file { text };
			bool fileExists = !file.fail();
			file.close();
			
			if (fileExists)
			{
				Commands::currentCommand = nullptr;
				exitMinibuffer("");

				Buffer* buffer = Buffer::getFromFilePath(text.substr(0, text.find(' ')));

				if (!buffer)
				{
					std::string filename = getFilenameFromPath(text.substr(0, text.find(' ')));
					buffer = new Buffer { BufferType::Text, filename, text.substr(0, text.find(' ')) };
				}

				FRAME->switchToBuffer(buffer);
				return true;
			}
			else
			{
				Commands::currentlyReading = MinibufferReading::Confirmation;
				BUFFER->data[0] += " [create? y/n] ";
				FRAME->point.col = BUFFER->data[0].size();
				FRAME->popupLines.clear();

				return false;
			}
		}
		else if (Commands::currentlyReading == MinibufferReading::Confirmation)
		{
			if (text.back() == 'y')
			{
				// This is the same as above
				std::string path = text.substr(0, text.find_last_of("["));
				std::string filename = getFilenameFromPath(path);
				Buffer* buffer = new Buffer { BufferType::Text, filename, path };

				Commands::currentCommand = nullptr;
				exitMinibuffer("");
				FRAME->switchToBuffer(buffer);

				return true;
			}
			else
			{
				Commands::currentlyReading = MinibufferReading::Path;
				
				// The +6 is for the "Path: " at the start of the string
				// The -1 is because we want to at the space before the bracket
				BUFFER->data[0] = BUFFER->data[0].substr(0, text.find_last_of("[") + 6 - 1);
				FRAME->point.col = BUFFER->data[0].size();
				FRAME->updatePopups();
				
				return false;
			}
		}
		else
		{
			printf("Error: Unhandled read type in findFile.\n");
			return true;
		}
	}
	else
	{
		Frame::minibufferFrame->makeActive();
		Commands::currentCommand = findFile;
		startReadingPath(window);

		return false;
	}
}

bool saveBuffer(Buffer* buffer, COMMAND_FUNC_SIG(commandName), Window& window, const std::string& text, bool requestPath)
{
	if (Commands::currentCommand == commandName && requestPath)
	{
		exitMinibuffer("");
		Commands::currentCommand = nullptr;

		if (text == "")
		{
			writeToMinibuffer("Error: No file path supplied.");
		}
		else
		{		
			if (text[text.size() - 1] == '/' || text[text.size() - 1] == '\\')
			{
				buffer->path = text + buffer->name;
			}
			else
			{
				buffer->path = text;
			}

			buffer->saveToFile();
			writeToMinibuffer("Saved \"" + buffer->path + "\"");
		}
		
		return true;
	}
	else
	{
		if (buffer->path != "")
		{
			exitMinibuffer("");
			buffer->saveToFile();
			writeToMinibuffer("Saved \"" + buffer->path + "\"");
		
			return true;
		}
		else if (requestPath)
		{
			Frame::minibufferFrame->makeActive();
			Commands::currentCommand = commandName;
			startReadingPath(window);

			return false;
		}
		else
		{
			return true;
		}
	}
}

DEFINE_COMMAND(saveCurrentBuffer)
{
	return saveBuffer(BUFFER, saveCurrentBuffer, window, text, true);
}

DEFINE_COMMAND(saveAllBuffers)
{
	// NOTE(fkp): This won't actually ask for a path, but will save
	// all file-visiting buffers.
	for (auto& bufferElement : Buffer::buffersMap)
	{
		saveBuffer(bufferElement.second, saveAllBuffers, window, text, false);
	}

	return true;
}

DEFINE_COMMAND(revertBuffer)
{
	exitMinibuffer("");
	BUFFER->revertToFile();

	return true;
}

DEFINE_COMMAND(lexBufferAsCpp)
{
	exitMinibuffer("");
	BUFFER->isUsingSyntaxHighlighting = true;
	BUFFER->lexer.lex(0, true);
	
	return true;
}

DEFINE_COMMAND(completeSuggestion)
{
	// This is a little hacky, but it is to get rid of the tab that
	// gets inserted when this is used as a keybind.
	if (FRAME->point.col > 0 &&
		BUFFER->data[FRAME->point.line][FRAME->point.col - 1] == '\t')
	{
		FRAME->backspaceChar();

		if (FRAME->popupLines.size() == 0)
		{
			FRAME->insertChar('\t');
		}
	}

	FRAME->completeSuggestion();	
	return false;
}

void centerSuggestions()
{
	// Copied from renderer.cpp
	unsigned int numberOfLines = FRAME->popupLines.size();
	if (numberOfLines > Renderer::popupMaxNumberOfLines) numberOfLines = Renderer::popupMaxNumberOfLines;

	// One unweildy if...
	if (!((FRAME->popupCurrentSuggestion >= FRAME->popupTargetTopLine &&
		   FRAME->popupCurrentSuggestion < FRAME->popupTargetTopLine + numberOfLines) ||
		  (FRAME->popupTargetTopLine + numberOfLines > FRAME->popupLines.size() &&
		   FRAME->popupCurrentSuggestion < (FRAME->popupTargetTopLine + numberOfLines) - FRAME->popupLines.size())))
	{
		int topLine = (int) FRAME->popupCurrentSuggestion - ((int) numberOfLines / 2);

		if (topLine < 0)
		{
			topLine += FRAME->popupLines.size();
		}
		
		FRAME->popupTargetTopLine = topLine;
	}
}

DEFINE_COMMAND(previousSuggestion)
{
	if (FRAME->popupLines.size() > 0)
	{
		if (FRAME->popupCurrentSuggestion == 0)
		{
			FRAME->popupCurrentSuggestion = FRAME->popupLines.size();
		}

		FRAME->popupCurrentSuggestion -= 1;
		centerSuggestions();
	}

	return false;
}

DEFINE_COMMAND(nextSuggestion)
{
	if (FRAME->popupLines.size() > 0)
	{
		FRAME->popupCurrentSuggestion += 1;

		if (FRAME->popupCurrentSuggestion >= FRAME->popupLines.size())
		{
			FRAME->popupCurrentSuggestion = 0;
		}

		centerSuggestions();
	}

	return false;
}

//
// NOTE(fkp): Project commands
//

DEFINE_COMMAND(saveProject)
{
	if (Commands::currentCommand)
	{
		exitMinibuffer("");
		Commands::currentCommand = nullptr;

		if (text == "")
		{
			writeToMinibuffer("Error: No file path supplied.");
		}
		else
		{		
			if (text[text.size() - 1] == '/' || text[text.size() - 1] == '\\')
			{
				writeToMinibuffer("Error: Specify project file, not directory.");
			}
			else
			{
				window.currentProject.saveToFile(text, window);
				writeToMinibuffer("Saved project to \"" + text + "\"");
			}
		}
		
		return true;
	}
	else
	{
		// Maybe have a save and save as?
		if (window.currentProject.currentPath != "")
		{
			exitMinibuffer("");
			window.currentProject.saveToFile(window.currentProject.currentPath, window);
			writeToMinibuffer("Saved project to \"" + window.currentProject.currentPath + "\"");
		
			return true;
		}
		else
		{
			Frame::minibufferFrame->makeActive();
			Commands::currentCommand = saveProject;
			startReadingPath(window);

			return false;
		}
	}
}

DEFINE_COMMAND(loadProject)
{
	// Mostly copied from saveProject() above
	if (Commands::currentCommand)
	{
		exitMinibuffer("");
		Commands::currentCommand = nullptr;

		if (text == "")
		{
			writeToMinibuffer("Error: No file path supplied.");
		}
		else
		{		
			if (text[text.size() - 1] == '/' || text[text.size() - 1] == '\\')
			{
				writeToMinibuffer("Error: Specify project file, not directory.");
			}
			else
			{
				window.currentProject.loadFromFile(text, window);
				writeToMinibuffer("Loaded project from \"" + text + "\"");
			}
		}
		
		return true;
	}
	else
	{
		Frame::minibufferFrame->makeActive();
		Commands::currentCommand = loadProject;
		startReadingPath(window);

		return false;
	}
}

DEFINE_COMMAND(compile)
{
	exitMinibuffer("");

	// One for one main frame and one for the minibuffer
	if (window.frames.size() == 2)
	{
		FRAME->split(true, window.renderer->currentFont);
	}
	else
	{
		window.moveToNextFrame();
	}

	saveAllBuffers(window, text);

	Buffer* compileBuffer = Buffer::get("*compilation*");
	
	if (compileBuffer == nullptr)
	{
		compileBuffer = new Buffer(BufferType::Text, "*compilation*", "__compile__.pe");
	}

	compileBuffer->isReadOnly = true;
	FRAME->switchToBuffer(compileBuffer);

	if (!window.currentProject.executeCompileCommand(compileBuffer))
	{
		writeToMinibuffer("Error: Compilation already in progress.");
	}
	
	return true;
}
