//  ===== Date Created: 15 April, 2020 ===== 

#if !defined(BUFFER_HPP)
#define BUFFER_HPP

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>

// NOTE(fkp): This is for DWORD, including <windows.h> gives errors
// for some reason.
#include <IntSafe.h>

#include "point.hpp"
#include "undo.hpp"
#include "lexer.hpp"

class Frame;

enum class BufferType
{
	MiniBuffer,
	Text,
};

class Buffer
{
public:
	inline static std::unordered_map<std::string, Buffer*> buffersMap;
	
	BufferType type;
	std::string name;
	std::string path;
	
	std::vector<std::string> data;
	Lexer lexer;
	bool isUsingSyntaxHighlighting = false;
	
	bool shouldAddToUndoInformation = true;
	std::deque<Action> undoInformation;
	unsigned int undoInformationPointer = 0;
	
	// The frame will take a copy of this when opened, and the last
	// frame to close this buffer will write its values in.
	Point lastPoint;
	unsigned int lastTopLine = 0;
	
public:
	Buffer(BufferType type, std::string name, std::string path);
	~Buffer();
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer(Buffer&& other);
	Buffer& operator=(Buffer&& other);
	
	static Buffer* get(const std::string& name);
	static Buffer* getFromFilePath(const std::string& path);
	
	void addActionToUndoBuffer(Action&& action);
	bool undo(Frame& frame);
	bool redo(Frame& frame);
	void saveToFile();

	// NOTE(fkp): Calls the standalone substrFromPoints() function
	// with the buffer's string.
	std::string substrFromPoints(const Point& start, const Point& end);
};

std::string substrFromPoints(const std::string& string, const Point& start, const Point& end, unsigned int offset);
Point getPointAtEndOfString(const std::string& string, unsigned int lineOffset);

#endif
