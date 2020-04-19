//  ===== Date Created: 15 April, 2020 ===== 

#if !defined(BUFFER_HPP)
#define BUFFER_HPP

#include <string>
#include <vector>

class Frame;

enum class BufferType
{
	MiniBuffer,
	Text,
};

class Buffer
{
public:
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

	// Utility
	// TODO(fkp): Currently only registers space, do other word boundaries
	unsigned int findWordBoundaryLeft(Frame& frame);
	unsigned int findWordBoundaryRight(Frame& frame);
	void moveColToTarget(Frame& frame);
};

#endif
