//  ===== Date Created: 19 April, 2020 ===== 

#if !defined(DEFAULT_KEY_BINDINGS_HPP)
#define DEFAULT_KEY_BINDINGS_HPP

#include "keymap.hpp"

void mapDefaultKeyBindings()
{
	// NOTE(fkp): KeyCombo constructor is in the order:
	// Key, Control, Shift, Alt, Windows
	
	KeyMap::bindKey({ Key::Enter, true }, "frameSplitVertically");
	KeyMap::bindKey({ Key::Enter, true, false, true }, "frameSplitHorizontally");
}

#endif
