// NOTE(fkp): This file is not to be compiled, it will be included in
// commands.cpp

#define DEFINE_COMMAND(name) bool name(Window& window, const std::string& text)
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
	BUFFER->backspaceChar(1);
	return false;
}

DEFINE_COMMAND(deleteChar)
{
	BUFFER->deleteChar(1);
	return false;
}

DEFINE_COMMAND(backspaceWord)
{
	BUFFER->backspaceChar(BUFFER->findWordBoundaryLeft());
	return false;
}

DEFINE_COMMAND(deleteWord)
{
	BUFFER->deleteChar(BUFFER->findWordBoundaryRight());
	return false;
}


//
// NOTE(fkp): Point movement commands
//

DEFINE_COMMAND(movePointLeftChar)
{
	BUFFER->movePointLeft(1);
	return false;
}

DEFINE_COMMAND(movePointRightChar)
{
	BUFFER->movePointRight(1);
	return false;
}

DEFINE_COMMAND(movePointLeftWord)
{
	BUFFER->movePointLeft(BUFFER->findWordBoundaryLeft());
	return false;
}

DEFINE_COMMAND(movePointRightWord)
{
	BUFFER->movePointLeft(BUFFER->findWordBoundaryRight());
	return false;
}

DEFINE_COMMAND(movePointLineUp)
{
	BUFFER->movePointUp();
	return false;
}

DEFINE_COMMAND(movePointLineDown)
{
	BUFFER->movePointDown();
	return false;
}

DEFINE_COMMAND(movePointHome)
{
	BUFFER->movePointHome();
	return false;
}

DEFINE_COMMAND(movePointEnd)
{
	BUFFER->movePointEnd();
	return false;
}

