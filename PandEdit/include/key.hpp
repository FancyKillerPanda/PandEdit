//  ===== Date Created: 02 July, 2020 ===== 

#if !defined(KEY_HPP)
#define KEY_HPP

#define NUMBER_OF_MODIFIER_KEYS 4

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

enum ModifierKey
{
	// TODO(fkp): Right now the left/right variants don't really do
	// anything specific.
	NO_MODIFIERS  = 0x00,
	
	LEFT_CONTROL  = 0x01,
	RIGHT_CONTROL = 0x02,
	KEY_CONTROL   = LEFT_CONTROL | RIGHT_CONTROL,
	
	LEFT_SHIFT    = 0x04,
	RIGHT_SHIFT   = 0x08,
	KEY_SHIFT     = LEFT_SHIFT | RIGHT_SHIFT,
	
	LEFT_ALT      = 0x10,
	RIGHT_ALT     = 0x20,
	KEY_ALT       = LEFT_ALT | RIGHT_ALT,
	
	KEY_WINDOWS       = 0x40,
};

#endif
