//  ===== Date Created: 15 April, 2020 ===== 

#if !defined(BUFFER_HPP)
#define BUFFER_HPP

#include <string>
#include <vector>
#include <unordered_map>

class Frame;

enum class BufferType
{
	MiniBuffer,
	Text,
};

class Buffer
{
public:
	static std::unordered_map<std::string, Buffer*> buffersMap;
	
	BufferType type;
	std::string name;
	
	std::vector<std::string> data;
	
	// The frame will take a copy of this when opened, and the last
	// frame to close this buffer will write its values in.
	unsigned int lastLine = 0;
	unsigned int lastCol = 0;
	unsigned int lastTargetCol = 0;

	// TODO(fkp): Make this a timer
	unsigned int pointFlashFrameCounter = 0;

public:
	Buffer(BufferType type, std::string name);
	~Buffer();
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer(Buffer&& other);
	Buffer& operator=(Buffer&& other);
	
	static Buffer* get(const std::string& name);
	
	// This is stuff that is common to all point manipulations
	void doCommonPointManipulationTasks();
	
	// Movement of the point
	void movePointLeft(Frame& frame, unsigned int num = 1);
	void movePointRight(Frame& frame, unsigned int num = 1);
	void movePointUp(Frame& frame);
	void movePointDown(Frame& frame);
	void movePointHome(Frame& frame);
	void movePointEnd(Frame& frame);
	
	// Manipulations at the point
	void insertChar(Frame& frame, char character);
	void backspaceChar(Frame& frame, unsigned int num = 1);
	void deleteChar(Frame& frame, unsigned int num = 1);
	void newLine(Frame& frame);
	void insertString(Frame& frame, const std::string& string);

	// Copy/cut/paste
	void pasteClipboard(Frame& frame);

	// Utility
	// TODO(fkp): Currently only registers space, do other word boundaries
	unsigned int findWordBoundaryLeft(Frame& frame);
	unsigned int findWordBoundaryRight(Frame& frame);
	void moveColToTarget(Frame& frame);
};

#endif
