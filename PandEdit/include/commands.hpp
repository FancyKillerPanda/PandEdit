//  ===== Date Created: 17 April, 2020 ===== 

#if !defined(COMMANDS_HPP)
#define COMMANDS_HPP

#include <unordered_map>
#include <string>

class Window;

class Commands
{
public:
	static std::unordered_map<std::string, bool (*)(Window&, const std::string&)> commandsMap;

public:
	static void executeCommand(Window& window, const std::string& commandText);
};

#endif
