// NOTE(fkp): This file is not to be compiled, it will be included in
// commands.cpp

#include "file_util.hpp"
#include "renderer.hpp"
#include "lexer.hpp"

#define DEFINE_COMMAND(name) bool name(Window& window, const std::string& text)
#define FRAME Frame::currentFrame
#define BUFFER Frame::currentFrame->currentBuffer

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

DEFINE_COMMAND(quit)
{
	window.isOpen = false;
	return true;
}

DEFINE_COMMAND(minibufferEnter)
{
	if (Frame::currentFrame != Frame::minibufferFrame)
	{
		Frame::minibufferFrame->makeActive();
		writeToMinibuffer("Execute: ");
		Commands::isReadingPath = false; // This shouldn't be necessary
	}
	return false;
}

DEFINE_COMMAND(minibufferQuit)
{
	exitMinibuffer("Quit");
	Frame::previousFrame->makeActive();
	Commands::currentCommand = nullptr;
	Commands::isReadingPath = false;
	
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
	if (BUFFER->type == BufferType::MiniBuffer && Commands::isReadingPath)
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
	if (Commands::currentCommand)
	{
		Commands::currentCommand = nullptr;
		exitMinibuffer("");
		
		Buffer* buffer = Buffer::get(text.substr(0, text.find(' ')));

		if (!buffer)
		{
			buffer = new Buffer { BufferType::Text, text.substr(0, text.find(' ')), "" };
		}
		
		FRAME->switchToBuffer(buffer);
		return true;
	}
	else
	{
		Frame::minibufferFrame->makeActive();
		writeToMinibuffer("Buffer: ");
		Commands::currentCommand = switchToBuffer;
		
		return false;
	}
}

DEFINE_COMMAND(destroyBuffer)
{
	if (Commands::currentCommand)
	{
		Commands::currentCommand = nullptr;
		exitMinibuffer("");

		std::string bufferName = text.substr(0, text.find(' '));
		FRAME->destroyBuffer(Buffer::get(bufferName));
		
		return true;
	}
	else
	{
		Frame::minibufferFrame->makeActive();
		// TODO(fkp): Show default buffer (minibuffer text improvements)
		writeToMinibuffer("Buffer: ");
		Commands::currentCommand = destroyBuffer;

		return false;
	}
}

// TODO(fkp): This command is very similar to switchToBuffer.
DEFINE_COMMAND(findFile)
{
	if (Commands::currentCommand)
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
		Frame::minibufferFrame->makeActive();
		Commands::currentCommand = findFile;
		
		std::string message = "Path: " + window.currentWorkingDirectory;
		Commands::isReadingPath = true;
		writeToMinibuffer(message);
		FRAME->updatePopups();

		return false;
	}
}

DEFINE_COMMAND(saveCurrentBuffer)
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
				BUFFER->path = text + BUFFER->name;
			}
			else
			{
				BUFFER->path = text;
			}

			BUFFER->saveToFile();
			writeToMinibuffer("Saved \"" + BUFFER->path + "\"");
		}
		
		return true;
	}
	else
	{
		if (BUFFER->path != "")
		{
			exitMinibuffer("");
			BUFFER->saveToFile();
			writeToMinibuffer("Saved \"" + BUFFER->path + "\"");
		
			return true;
		}
		else
		{
			Frame::minibufferFrame->makeActive();
			Commands::currentCommand = saveCurrentBuffer;
			
			std::string message = "Path: " + window.currentWorkingDirectory;
			Commands::isReadingPath = true;
			writeToMinibuffer(message);
			FRAME->updatePopups();

			return false;
		}
	}
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
