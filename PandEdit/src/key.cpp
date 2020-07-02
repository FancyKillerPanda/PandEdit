//  ===== Date Created: 02 July, 2020 ===== 

#include "key.hpp"

const char* toCString(Key key)
{
	switch (key)
	{
	case Key::Control:		return "Control";
	case Key::Shift:		return "Shift";
	case Key::Alt:			return "Alt";
	case Key::Windows:		return "Windows";

	case Key::Enter:		return "Enter";
	case Key::Tab:			return "Tab";
	case Key::Insert:		return "Insert";
	case Key::Escape:		return "Escape";
	case Key::Menu:			return "Menu";

	case Key::Backspace:	return "Backspace";
	case Key::Delete:		return "Delete";
	case Key::Space:		return "Space";
	
	case Key::CapsLock:		return "CapsLock";
	case Key::NumLock:		return "NumLock";
	case Key::ScrollLock:	return "ScrollLock";
	
	case Key::PageUp:		return "PageUp";
	case Key::PageDown:		return "PageDown";
	case Key::Home:			return "Home";
	case Key::End:			return "End";

	case Key::LeftArrow:	return "LeftArrow";
	case Key::RightArrow:	return "RightArrow";
	case Key::UpArrow:		return "UpArrow";
	case Key::DownArrow:	return "DownArrow";	
	
	case Key::_0:			return "_0";
	case Key::_1:			return "_1";
	case Key::_2:			return "_2";
	case Key::_3:			return "_3";
	case Key::_4:			return "_4";
	case Key::_5:			return "_5";
	case Key::_6:			return "_6";
	case Key::_7:			return "_7";
	case Key::_8:			return "_8";
	case Key::_9:			return "_9";

	case Key::A:			return "A";
	case Key::B:			return "B";
	case Key::C:			return "C";
	case Key::D:			return "D";
	case Key::E:			return "E";
	case Key::F:			return "F";
	case Key::G:			return "G";
	case Key::H:			return "H";
	case Key::I:			return "I";
	case Key::J:			return "J";
	case Key::K:			return "K";
	case Key::L:			return "L";
	case Key::M:			return "M";
	case Key::N:			return "N";
	case Key::O:			return "O";
	case Key::P:			return "P";
	case Key::Q:			return "Q";
	case Key::R:			return "R";
	case Key::S:			return "S";
	case Key::T:			return "T";
	case Key::U:			return "U";
	case Key::V:			return "V";
	case Key::W:			return "W";
	case Key::X:			return "X";
	case Key::Y:			return "Y";
	case Key::Z:			return "Z";
		
	case Key::NumPad0:		return "NumPad0";
	case Key::NumPad1:		return "NumPad1";
	case Key::NumPad2:		return "NumPad2";
	case Key::NumPad3:		return "NumPad3";
	case Key::NumPad4:		return "NumPad4";
	case Key::NumPad5:		return "NumPad5";
	case Key::NumPad6:		return "NumPad6";
	case Key::NumPad7:		return "NumPad7";
	case Key::NumPad8:		return "NumPad8";
	case Key::NumPad9:		return "NumPad9";

	case Key::F1:			return "F1";
	case Key::F2:			return "F2";
	case Key::F3:			return "F3";
	case Key::F4:			return "F4";
	case Key::F5:			return "F5";
	case Key::F6:			return "F6";
	case Key::F7:			return "F7";
	case Key::F8:			return "F8";
	case Key::F9:			return "F9";
	case Key::F10:			return "F10";
	case Key::F11:			return "F11";
	case Key::F12:			return "F12";
	case Key::F13:			return "F13";
	case Key::F14:			return "F14";
	case Key::F15:			return "F15";
	case Key::F16:			return "F16";
	case Key::F17:			return "F17";
	case Key::F18:			return "F18";
	case Key::F19:			return "F19";
	case Key::F20:			return "F20";
	case Key::F21:			return "F21";
	case Key::F22:			return "F22";
	case Key::F23:			return "F23";
	case Key::F24:			return "F24";

	case Key::Plus:			return "Plus";
	case Key::Hyphen:		return "Hyphen";
	case Key::Comma:		return "Comma";
	case Key::Period:		return "Period";
	case Key::Semicolon:	return "Semicolon";
	case Key::ForwardSlash:	return "ForwardSlash";
	case Key::Backtick:		return "Backtick";
	case Key::OpenBracket:	return "OpenBracket";
	case Key::BackSlash:	return "BackSlash";
	case Key::CloseBracket:	return "CloseBracket";
	case Key::Apostrophe:	return "Apostrophe";
	
	case Key::Unknown:
	case Key::Count:
	default:
		return "Unknown";
	}
}
