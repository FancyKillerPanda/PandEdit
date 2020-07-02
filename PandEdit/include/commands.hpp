//  ===== Date Created: 17 April, 2020 ===== 

#if !defined(COMMANDS_HPP)
#define COMMANDS_HPP

#include <unordered_map>
#include <string>

#define COMMAND_FUNC_SIG(...) bool (*__VA_ARGS__)(Window&, const std::string&)

enum class MinibufferReading
{
	None,
	Command,
	Path,
	BufferName,
};

class Window;

class Commands
{
public:
	// Essential commands can be used while another command is in progress
	static std::unordered_map<std::string, COMMAND_FUNC_SIG()> essentialCommandsMap;
	static std::unordered_map<std::string, COMMAND_FUNC_SIG()> nonEssentialCommandsMap;
	inline static COMMAND_FUNC_SIG(currentCommand) = nullptr;
	inline static std::string lastCommand = "";
	inline static MinibufferReading currentlyReading = MinibufferReading::None;

public:
	static void executeCommand(Window& window, const std::string& commandText, bool shortcut);
};

#endif
