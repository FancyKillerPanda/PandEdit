//  ===== Date Created: 02 July, 2020 ===== 

#include "key_combo.hpp"

KeyCombo::KeyCombo(Key key, int modifiers)
	: mainKey(key), modifiers(modifiers)
{
}

bool KeyCombo::operator==(const KeyCombo& other) const
{
	return mainKey == other.mainKey && modifiers == other.modifiers;
}
