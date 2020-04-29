//  ===== Date Created: 18 April, 2020 ===== 

#if !defined(KEYBINDINGS_HPP)
#define KEYBINDINGS_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <windows.h>

enum class Key
{
	// TODO(fkp): Left/right versions
	Control,
	Shift,
	Alt,
	Windows,

	Enter,
	Tab,
	Insert,
	Escape,
	Menu,

	Backspace,
	Delete,
	Space,
	
	CapsLock,
	NumLock,
	ScrollLock,
	
	PageUp,
	PageDown,
	Home,
	End,

	LeftArrow,
	RightArrow,
	UpArrow,
	DownArrow,	
	
	_0, _1, _2, _3, _4, _5, _6, _7, _8, _9,
	A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

	NumPad0,
	NumPad1,
	NumPad2,
	NumPad3,
	NumPad4,
	NumPad5,
	NumPad6,
	NumPad7,
	NumPad8,
	NumPad9,

	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
	F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24,

	Plus,
	Hyphen,
	Comma,
	Period,
	Semicolon,
	ForwardSlash,
	Backtick,
	OpenBracket,
	BackSlash,
	CloseBracket,
	Apostrophe,
	
	Unknown,
	Count
};

struct KeyCombo
{
	Key mainKey;
	bool controlPressed;
	bool shiftPressed;
	bool altPressed;
	bool windowsPressed;

	KeyCombo() = default;	
	KeyCombo(Key key, bool control = false, bool shift = false, bool alt = false, bool windows = false)
		: mainKey(key), controlPressed(control), shiftPressed(shift), altPressed(alt), windowsPressed(windows)
	{
	}

	bool operator==(const KeyCombo& other) const
	{
		bool keyEqual		= mainKey == other.mainKey;
		bool controlEqual	= controlPressed == other.controlPressed;
		bool shiftEqual		= shiftPressed == other.shiftPressed;
		bool altEqual		= altPressed == other.altPressed;
		bool windowsEqual	= windowsPressed == other.windowsPressed;
		
		return keyEqual && controlEqual && shiftEqual && altEqual && windowsEqual;
	}
};

struct KeyComboHash
{
	std::size_t operator()(const KeyCombo combo) const
	{
		return (std::size_t) combo.mainKey;
	}
};

class Window;

class KeyMap
{
public:
	// TODO(fkp): This will need to increase when left/right modifiers are done
	inline static bool modifiersPressed[4] {};
	inline static std::unordered_map<KeyCombo, std::vector<std::string>, KeyComboHash> keyMap;
	
public:
	static void bindKey(KeyCombo keyCombo, std::string function);
	static void registerKeyPress(Window& window, Key key);
	static void registerKeyRelease(Key key);
	
	static Key convertWin32CodeToKey(WPARAM code);
};

#endif
