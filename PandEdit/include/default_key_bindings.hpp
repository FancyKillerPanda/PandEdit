//  ===== Date Created: 19 April, 2020 ===== 

#if !defined(DEFAULT_KEY_BINDINGS_HPP)
#define DEFAULT_KEY_BINDINGS_HPP

#include "keymap.hpp"

void mapDefaultKeyBindings()
{
	// NOTE(fkp): KeyCombo constructor is in the order:
	// Key, Control, Shift, Alt, Windows
	
	KeyMap::bindKey({ Key::X, false, false, true }, "minibufferEnter");
	KeyMap::bindKey({ Key::G, true }, "minibufferQuit");
	
	KeyMap::bindKey({ Key::_2, true }, "frameSplitVertically"); // TODO(fkp): Remap later
	KeyMap::bindKey({ Key::_3, true }, "frameSplitHorizontally"); // TODO(fkp): Remap later
	KeyMap::bindKey({ Key::W, false, false, true }, "frameMoveNext");
	KeyMap::bindKey({ Key::W, false, true, true }, "frameMovePrevious");
	
	KeyMap::bindKey({ Key::Backspace }, "backspaceChar");
	KeyMap::bindKey({ Key::Delete }, "deleteChar");
	KeyMap::bindKey({ Key::Backspace, true }, "backspaceWord");
	KeyMap::bindKey({ Key::Delete, true }, "deleteWord");
	
	KeyMap::bindKey({ Key::LeftArrow }, "movePointLeftChar");
	KeyMap::bindKey({ Key::RightArrow }, "movePointRightChar");
	KeyMap::bindKey({ Key::LeftArrow, true }, "movePointLeftWord");
	KeyMap::bindKey({ Key::RightArrow, true }, "movePointRightWord");
	KeyMap::bindKey({ Key::UpArrow }, "movePointLineUp");
	KeyMap::bindKey({ Key::DownArrow }, "movePointLineDown");
	KeyMap::bindKey({ Key::Home }, "movePointHome");
	KeyMap::bindKey({ Key::End }, "movePointEnd");
	KeyMap::bindKey({ Key::Space, true }, "setMark");
	KeyMap::bindKey({ Key::Semicolon, false, false, true }, "swapPointAndMark");

	KeyMap::bindKey({ Key::C, true }, "copyRegion");
	KeyMap::bindKey({ Key::V, true }, "paste");
	KeyMap::bindKey({ Key::V, false, false, true }, "pastePop");
	
	KeyMap::bindKey({ Key::B, false, false, true }, "switchToBuffer");
}

#endif
