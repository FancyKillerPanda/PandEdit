// NOTE(fkp): This file is not to be compiled, it will be included in
// commands.cpp

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
	return true;
}


//
// NOTE(fkp): Frame commands
//

DEFINE_COMMAND(frameSplitVertically)
{
	exitMinibuffer();
	window.splitCurrentFrameVertically();
	return true;
}

DEFINE_COMMAND(frameSplitHorizontally)
{
	exitMinibuffer();
	window.splitCurrentFrameHorizontally();
	return true;
}

DEFINE_COMMAND(frameMoveNext)
{
	exitMinibuffer();
	window.moveToNextFrame();
	return true;	
}

DEFINE_COMMAND(frameMovePrevious)
{
	exitMinibuffer();
	window.moveToNextFrame(false);
	return true;	
}


//
// NOTE(fkp): Text manipulation commands
//

DEFINE_COMMAND(backspaceChar)
{
	BUFFER->backspaceChar(*FRAME, 1);
	return false;
}

DEFINE_COMMAND(deleteChar)
{
	BUFFER->deleteChar(*FRAME, 1);
	return false;
}

DEFINE_COMMAND(backspaceWord)
{
	BUFFER->backspaceChar(*FRAME, BUFFER->findWordBoundaryLeft(*FRAME));
	return false;
}

DEFINE_COMMAND(deleteWord)
{
	BUFFER->deleteChar(*FRAME, BUFFER->findWordBoundaryRight(*FRAME));
	return false;
}


//
// NOTE(fkp): Point movement commands
//

DEFINE_COMMAND(movePointLeftChar)
{
	BUFFER->movePointLeft(*FRAME, 1);
	return false;
}

DEFINE_COMMAND(movePointRightChar)
{
	BUFFER->movePointRight(*FRAME, 1);
	return false;
}

DEFINE_COMMAND(movePointLeftWord)
{
	BUFFER->movePointLeft(*FRAME, BUFFER->findWordBoundaryLeft(*FRAME));
	return false;
}

DEFINE_COMMAND(movePointRightWord)
{
	BUFFER->movePointLeft(*FRAME, BUFFER->findWordBoundaryRight(*FRAME));
	return false;
}

DEFINE_COMMAND(movePointLineUp)
{
	BUFFER->movePointUp(*FRAME);
	return false;
}

DEFINE_COMMAND(movePointLineDown)
{
	BUFFER->movePointDown(*FRAME);
	return false;
}

DEFINE_COMMAND(movePointHome)
{
	BUFFER->movePointHome(*FRAME);
	return false;
}

DEFINE_COMMAND(movePointEnd)
{
	BUFFER->movePointEnd(*FRAME);
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


//
// NOTE(fkp): Copy/cut/paste
//

DEFINE_COMMAND(copyRegion)
{
	BUFFER->copyRegion(*FRAME);
	return false;
}

DEFINE_COMMAND(paste)
{
	BUFFER->paste(*FRAME);
	return false;
}

DEFINE_COMMAND(pastePop)
{
	if (Commands::lastCommand == "paste")
	{
		BUFFER->pastePop(*FRAME);
	}

	return false;
}

DEFINE_COMMAND(deleteRegion)
{
	FRAME->deleteTextPointToMark();
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
			buffer = new Buffer { BufferType::Text, text.substr(0, text.find(' ')) };
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
