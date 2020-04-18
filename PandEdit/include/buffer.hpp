//  ===== Date Created: 15 April, 2020 ===== 

#if !defined(BUFFER_HPP)
#define BUFFER_HPP

#include <string>
#include <vector>

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
	
	unsigned int line = 0;
	unsigned int col = 0;
	unsigned int targetCol = 0;

	// TODO(fkp): Make this a timer
	unsigned int pointFlashFrameCounter = 0;

public:
	Buffer(BufferType type, std::string name);

	// This is stuff that is common to all point manipulations
	void doCommonPointManipulationTasks();
	
	// Movement of the point
	void movePointLeft(unsigned int num = 1);
	void movePointRight(unsigned int num = 1);
	void movePointUp();
	void movePointDown();
	void movePointHome();
	void movePointEnd();
	
	// Manipulations at the point
	void insertChar(char character);
	void backspaceChar(unsigned int num = 1);
	void deleteChar(unsigned int num = 1);
	void newLine();

	// Utility
	// TODO(fkp): Currently only registers space, do other word boundaries
	unsigned int findWordBoundaryLeft();
	unsigned int findWordBoundaryRight();
	void moveColToTarget();
};

#endif
