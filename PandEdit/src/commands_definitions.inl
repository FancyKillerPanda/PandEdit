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
	}
	return false;
}

DEFINE_COMMAND(minibufferQuit)
{
	exitMinibuffer("Quit");
	Frame::previousFrame->makeActive();
	Commands::currentCommand = nullptr;
	
	return true;
}


//
// NOTE(fkp): Frame commands
//

DEFINE_COMMAND(frameSplitVertically)
{
	exitMinibuffer("");
	window.splitCurrentFrameVertically();
	return true;
}

DEFINE_COMMAND(frameSplitHorizontally)
{
	exitMinibuffer("");
	window.splitCurrentFrameHorizontally();
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
	FRAME->movePointUp(window.renderer->currentFont);
	return false;
}

DEFINE_COMMAND(movePointLineDown)
{
	FRAME->movePointDown(window.renderer->currentFont);
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
	FRAME->movePointToBufferStart(window.renderer->currentFont);
	return false;
}

DEFINE_COMMAND(movePointToBufferEnd)
{
	FRAME->movePointToBufferEnd(window.renderer->currentFont);
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

	return false;
}

DEFINE_COMMAND(pageUp)
{
	// Minibuffer shouldn't have scrolling
	if (BUFFER->type != BufferType::MiniBuffer)
	{
		FRAME->moveView(-(FRAME->getNumberOfLines(window.renderer->currentFont) - 3), true);
	}

	return false;
}

DEFINE_COMMAND(pageDown)
{
	// Minibuffer shouldn't have scrolling
	if (BUFFER->type != BufferType::MiniBuffer)
	{
		FRAME->moveView(FRAME->getNumberOfLines(window.renderer->currentFont) - 3, true);
	}

	return false;
}

DEFINE_COMMAND(centerPoint)
{
	// Minibuffer shouldn't have scrolling
	if (BUFFER->type != BufferType::MiniBuffer)
	{
		FRAME->centerPoint(window.renderer->currentFont);
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
		writeToMinibuffer(message);

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
			writeToMinibuffer(message);

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
