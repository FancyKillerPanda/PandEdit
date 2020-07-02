//  ===== Date Created: 02 July, 2020 ===== 

#include "key_combo.hpp"

KeyCombo::KeyCombo(Key key, int modifiers)
	: mainKey(key), modifiers(modifiers)
{
}

std::string KeyCombo::str()
{
	std::string result;

	if (modifiers & KEY_CONTROL) result += "Ctrl+";
	if (modifiers & KEY_ALT) result += "Alt+";
	if (modifiers & KEY_SHIFT) result += "Shift+";
	if (modifiers & KEY_WINDOWS) result += "Win+";
	result += toCString(mainKey);

	return result;
}

bool KeyCombo::operator==(const KeyCombo& other) const
{
	return mainKey == other.mainKey && modifiers == other.modifiers;
}
