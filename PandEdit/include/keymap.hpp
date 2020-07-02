//  ===== Date Created: 18 April, 2020 ===== 

#if !defined(KEYBINDINGS_HPP)
#define KEYBINDINGS_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <windows.h>

#include "key.hpp"
#include "key_combo.hpp"

class Window;

class KeyMap
{
public:
	// TODO(fkp): This will need to increase when left/right modifiers are done
	inline static bool modifiersPressed[NUMBER_OF_MODIFIER_KEYS] {};
	inline static std::unordered_map<KeyCombo, std::vector<std::string>, KeyComboHash> keyMap;
	
public:
	static void bindKey(KeyCombo keyCombo, std::string function);
	static void registerKeyPress(Window& window, Key key);
	static void registerKeyRelease(Key key);
	static void clearModifiers();
	
	static Key convertWin32CodeToKey(WPARAM code);
};

#endif
