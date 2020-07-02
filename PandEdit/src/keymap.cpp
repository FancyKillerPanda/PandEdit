//  ===== Date Created: 18 April, 2020 ===== 

#include "keymap.hpp"
#include "window.hpp"
#include "commands.hpp"

void KeyMap::bindKey(KeyCombo keyCombo, std::string function)
{
	auto result = keyMap.find(keyCombo);
	
	if (result != keyMap.end())
	{
		result->second.push_back(std::move(function));
	}
	else
	{
		keyMap.emplace(keyCombo, std::vector<std::string> { std::move(function) });
	}
}

void KeyMap::registerKeyPress(Window& window, Key key)
{
	if (key == Key::Control || key == Key::Shift || key == Key::Alt || key == Key::Windows)
	{
		modifiersPressed[(unsigned int) key] = true;
	}
	else
	{
		// TODO(fkp): Find a way to get rid of all those if statements
		KeyCombo combo;
		combo.mainKey = key;
		combo.modifiers = NO_MODIFIERS;
		if (modifiersPressed[(unsigned int) Key::Control]) combo.modifiers |= KEY_CONTROL;
		if (modifiersPressed[(unsigned int) Key::Shift]) combo.modifiers |= KEY_SHIFT;
		if (modifiersPressed[(unsigned int) Key::Alt]) combo.modifiers |= KEY_ALT;
		if (modifiersPressed[(unsigned int) Key::Windows]) combo.modifiers |= KEY_WINDOWS;
		
		auto result = keyMap.find(combo);

		if (result != keyMap.end())
		{
			for (const std::string& command : result->second)
			{
				Commands::executeCommand(window, command, true);
			}
		}
		/* TODO(fkp): Implement this
		else
		{
			writeToMinibuffer(combo.str() + " is undefined.");
		}
		*/
	}
}

void KeyMap::registerKeyRelease(Key key)
{
	if (key == Key::Control || key == Key::Shift || key == Key::Alt || key == Key::Windows)
	{
		modifiersPressed[(unsigned int) key] = false;
	}
}

void KeyMap::clearModifiers()
{
	for (int i = 0; i < NUMBER_OF_MODIFIER_KEYS; i++)
	{
		modifiersPressed[i] = false;
	}
}

Key KeyMap::convertWin32CodeToKey(WPARAM code)
{
	switch (code)
	{
	case VK_CONTROL:	return Key::Control;
	case VK_SHIFT:		return Key::Shift;
	case VK_MENU:		return Key::Alt;
	case VK_LWIN:		return Key::Windows;

	case VK_RETURN:		return Key::Enter;
	case VK_TAB:		return Key::Tab;
	case VK_INSERT:		return Key::Insert;
	case VK_ESCAPE:		return Key::Escape;
	case VK_APPS:		return Key::Menu;

	case VK_BACK:		return Key::Backspace;
	case VK_DELETE:		return Key::Delete;
	case VK_SPACE:		return Key::Space;

	case VK_CAPITAL:	return Key::CapsLock;
	case VK_NUMLOCK:	return Key::NumLock;
	case VK_SCROLL:		return Key::ScrollLock;

	case VK_PRIOR:		return Key::PageUp;
	case VK_NEXT:		return Key::PageDown;
	case VK_HOME:		return Key::Home;
	case VK_END:		return Key::End;

	case VK_LEFT:		return Key::LeftArrow;
	case VK_RIGHT:		return Key::RightArrow;
	case VK_UP:			return Key::UpArrow;
	case VK_DOWN:		return Key::DownArrow;
		
	case 0x30:			return Key::_0;
	case 0x31:			return Key::_1;
	case 0x32:			return Key::_2;
	case 0x33:			return Key::_3;
	case 0x34:			return Key::_4;
	case 0x35:			return Key::_5;
	case 0x36:			return Key::_6;
	case 0x37:			return Key::_7;
	case 0x38:			return Key::_8;
	case 0x39:			return Key::_9;
		
	case 0x41:			return Key::A;
	case 0x42:			return Key::B;
	case 0x43:			return Key::C;
	case 0x44:			return Key::D;
	case 0x45:			return Key::E;
	case 0x46:			return Key::F;
	case 0x47:			return Key::G;
	case 0x48:			return Key::H;
	case 0x49:			return Key::I;
	case 0x4A:			return Key::J;
	case 0x4B:			return Key::K;
	case 0x4C:			return Key::L;
	case 0x4D:			return Key::M;
	case 0x4E:			return Key::N;
	case 0x4F:			return Key::O;
	case 0x50:			return Key::P;
	case 0x51:			return Key::Q;
	case 0x52:			return Key::R;
	case 0x53:			return Key::S;
	case 0x54:			return Key::T;
	case 0x55:			return Key::U;
	case 0x56:			return Key::V;
	case 0x57:			return Key::W;
	case 0x58:			return Key::X;
	case 0x59:			return Key::Y;
	case 0x5A:			return Key::Z;

	case VK_NUMPAD0:	return Key::NumPad0;
	case VK_NUMPAD1:	return Key::NumPad1;
	case VK_NUMPAD2:	return Key::NumPad2;
	case VK_NUMPAD3:	return Key::NumPad3;
	case VK_NUMPAD4:	return Key::NumPad4;
	case VK_NUMPAD5:	return Key::NumPad5;
	case VK_NUMPAD6:	return Key::NumPad6;
	case VK_NUMPAD7:	return Key::NumPad7;
	case VK_NUMPAD8:	return Key::NumPad8;
	case VK_NUMPAD9:	return Key::NumPad9;

	case VK_F1:			return Key::F1;
	case VK_F2:			return Key::F2;
	case VK_F3:			return Key::F3;
	case VK_F4:			return Key::F4;
	case VK_F5:			return Key::F5;
	case VK_F6:			return Key::F6;
	case VK_F7:			return Key::F7;
	case VK_F8:			return Key::F8;
	case VK_F9:			return Key::F9;
	case VK_F10:		return Key::F10;
	case VK_F11:		return Key::F11;
	case VK_F12:		return Key::F12;
	case VK_F13:		return Key::F13;
	case VK_F14:		return Key::F14;
	case VK_F15:		return Key::F15;
	case VK_F16:		return Key::F16;
	case VK_F17:		return Key::F17;
	case VK_F18:		return Key::F18;
	case VK_F19:		return Key::F19;
	case VK_F20:		return Key::F20;
	case VK_F21:		return Key::F21;
	case VK_F22:		return Key::F22;
	case VK_F23:		return Key::F23;
	case VK_F24:		return Key::F24;

	case VK_OEM_PLUS:	return Key::Plus;
	case VK_OEM_MINUS:	return Key::Hyphen;
	case VK_OEM_COMMA:	return Key::Comma;
	case VK_OEM_PERIOD:	return Key::Period;
	case VK_OEM_1:		return Key::Semicolon;
	case VK_OEM_2:		return Key::ForwardSlash;
	case VK_OEM_3:		return Key::Backtick;
	case VK_OEM_4:		return Key::OpenBracket;
	case VK_OEM_5:		return Key::BackSlash;
	case VK_OEM_6:		return Key::CloseBracket;
	case VK_OEM_7:		return Key::Apostrophe;

	default:			return Key::Unknown;
	}
}
