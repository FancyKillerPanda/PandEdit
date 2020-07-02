//  ===== Date Created: 02 July, 2020 ===== 

#if !defined(KEY_COMBO_HPP)
#define KEY_COMBO_HPP

#include <cstddef>
#include <string>

#include "key.hpp"

class KeyCombo
{
public:
	Key mainKey;
	int modifiers = NO_MODIFIERS;

public:
	KeyCombo() = default;
	KeyCombo(Key key, int modifiers = NO_MODIFIERS);

	std::string str();
	bool operator==(const KeyCombo& other) const;
};

struct KeyComboHash
{
	std::size_t operator()(const KeyCombo combo) const
	{
		return (std::size_t) combo.mainKey;
	}
};

#endif
