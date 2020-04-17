//  ===== Date Created: 17 April, 2020 ===== 

#if !defined(COMMANDS_HPP)
#define COMMANDS_HPP

#include <unordered_map>
#include <string>

class Commands
{
public:
	static std::unordered_map<std::string, bool (*)(const std::string& text)> commandsMap;

public:
	static void executeCommand(const std::string& commandText);
};

#endif
