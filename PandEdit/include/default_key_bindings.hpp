//  ===== Date Created: 19 April, 2020 ===== 

#if !defined(DEFAULT_KEY_BINDINGS_HPP)
#define DEFAULT_KEY_BINDINGS_HPP

#include "keymap.hpp"

void mapDefaultKeyBindings()
{
	// NOTE(fkp): KeyCombo constructor is in the order:
	// Key, Control, Shift, Alt, Windows

	KeyMap::bindKey({ Key::F4, false, false, true }, "quit");
	KeyMap::bindKey({ Key::X, false, false, true }, "minibufferEnter");
	KeyMap::bindKey({ Key::G, true }, "minibufferQuit");

	KeyMap::bindKey({ Key::_4, true }, "lexBufferAsC++"); // NOTE(fkp): Temporary
	
	KeyMap::bindKey({ Key::_2, true }, "frameSplitVertically"); // TODO(fkp): Remap later
	KeyMap::bindKey({ Key::_3, true }, "frameSplitHorizontally"); // TODO(fkp): Remap later
	KeyMap::bindKey({ Key::W, false, false, true }, "frameMoveNext");
	KeyMap::bindKey({ Key::W, false, true, true }, "frameMovePrevious");
	
	KeyMap::bindKey({ Key::Backspace }, "backspaceChar");
	KeyMap::bindKey({ Key::Backspace, false, true }, "backspaceChar");
	KeyMap::bindKey({ Key::Delete }, "deleteChar");
	KeyMap::bindKey({ Key::Backspace, true }, "backspaceWord");
	KeyMap::bindKey({ Key::Delete, true }, "deleteWord");
	KeyMap::bindKey({ Key::K, true }, "deleteRestOfLine");
	
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

	KeyMap::bindKey({ Key::PageUp }, "pageUp");
	KeyMap::bindKey({ Key::PageDown }, "pageDown");
	KeyMap::bindKey({ Key::L, true }, "centerPoint");

	KeyMap::bindKey({ Key::C, true }, "copyRegion");
	KeyMap::bindKey({ Key::V, true }, "paste");
	KeyMap::bindKey({ Key::V, false, false, true }, "pastePop");
	KeyMap::bindKey({ Key::W, true }, "deleteRegion");
	KeyMap::bindKey({ Key::Z, true }, "undo");
	KeyMap::bindKey({ Key::Y, true }, "redo");
	
	KeyMap::bindKey({ Key::B, false, false, true }, "switchToBuffer");
	KeyMap::bindKey({ Key::F, false, false, true }, "findFile");
	KeyMap::bindKey({ Key::S, false, false, true }, "saveCurrentBuffer");
}

#endif
