//  ===== Date Created: 19 April, 2020 ===== 

#if !defined(DEFAULT_KEY_BINDINGS_HPP)
#define DEFAULT_KEY_BINDINGS_HPP

#include "keymap.hpp"

void mapDefaultKeyBindings()
{
	// NOTE(fkp): KeyCombo constructor is in the order:
	// Key, Control, Shift, Alt, Windows

	KeyMap::bindKey({ Key::F4, KEY_ALT }, "quit");
	KeyMap::bindKey({ Key::X, KEY_ALT }, "minibufferEnter");
	KeyMap::bindKey({ Key::G, KEY_CONTROL }, "minibufferQuit");
	KeyMap::bindKey({ Key::Insert }, "toggleOverwriteMode");

	KeyMap::bindKey({ Key::_4, KEY_CONTROL }, "lexBufferAsC++"); // NOTE(fkp): Temporary
	
	KeyMap::bindKey({ Key::_2, KEY_CONTROL }, "frameSplitVertically"); // TODO(fkp): Remap later
	KeyMap::bindKey({ Key::_3, KEY_CONTROL }, "frameSplitHorizontally"); // TODO(fkp): Remap later
	KeyMap::bindKey({ Key::_0, KEY_CONTROL }, "frameDestroy"); // TODO(fkp): Remap later
	KeyMap::bindKey({ Key::W, KEY_ALT }, "frameMoveNext");
	KeyMap::bindKey({ Key::W, KEY_ALT | KEY_SHIFT }, "frameMovePrevious");
	
	KeyMap::bindKey({ Key::Backspace }, "backspaceChar");
	KeyMap::bindKey({ Key::Backspace, KEY_SHIFT }, "backspaceCharExtra");
	KeyMap::bindKey({ Key::Delete }, "deleteChar");
	KeyMap::bindKey({ Key::Backspace, KEY_CONTROL }, "backspaceWord");
	KeyMap::bindKey({ Key::Delete, KEY_CONTROL }, "deleteWord");
	KeyMap::bindKey({ Key::K, KEY_CONTROL }, "deleteRestOfLine");
	
	KeyMap::bindKey({ Key::LeftArrow }, "movePointLeftChar");
	KeyMap::bindKey({ Key::RightArrow }, "movePointRightChar");
	KeyMap::bindKey({ Key::LeftArrow, KEY_CONTROL }, "movePointLeftWord");
	KeyMap::bindKey({ Key::RightArrow, KEY_CONTROL }, "movePointRightWord");
	KeyMap::bindKey({ Key::UpArrow }, "movePointLineUp");
	KeyMap::bindKey({ Key::DownArrow }, "movePointLineDown");
	KeyMap::bindKey({ Key::Home }, "movePointHome");
	KeyMap::bindKey({ Key::End }, "movePointEnd");
	KeyMap::bindKey({ Key::Home, KEY_CONTROL }, "movePointToBufferStart");
	KeyMap::bindKey({ Key::End, KEY_CONTROL }, "movePointToBufferEnd");

	KeyMap::bindKey({ Key::Space, KEY_CONTROL }, "setMark");
	KeyMap::bindKey({ Key::Semicolon, KEY_ALT }, "swapPointAndMark");

	KeyMap::bindKey({ Key::PageUp }, "pageUp");
	KeyMap::bindKey({ Key::PageDown }, "pageDown");
	KeyMap::bindKey({ Key::L, KEY_CONTROL }, "centerPoint");

	KeyMap::bindKey({ Key::C, KEY_CONTROL }, "copyRegion");
	KeyMap::bindKey({ Key::V, KEY_CONTROL }, "paste");
	KeyMap::bindKey({ Key::V, KEY_ALT }, "pastePop");
	KeyMap::bindKey({ Key::W, KEY_CONTROL }, "deleteRegion");
	KeyMap::bindKey({ Key::Z, KEY_CONTROL }, "undo");
	KeyMap::bindKey({ Key::Y, KEY_CONTROL }, "redo");
	
	KeyMap::bindKey({ Key::B, KEY_ALT }, "switchToBuffer");
	KeyMap::bindKey({ Key::K, KEY_ALT }, "destroyBuffer");
	KeyMap::bindKey({ Key::F, KEY_ALT }, "findFile");
	KeyMap::bindKey({ Key::S, KEY_ALT }, "saveCurrentBuffer");

	KeyMap::bindKey({ Key::Tab }, "completeSuggestion");
	KeyMap::bindKey({ Key::UpArrow, KEY_CONTROL }, "previousSuggestion");
	KeyMap::bindKey({ Key::DownArrow, KEY_CONTROL }, "nextSuggestion");

	KeyMap::bindKey({ Key::M, KEY_ALT }, "compile");
}

#endif
