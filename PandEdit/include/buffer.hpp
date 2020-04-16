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
	
	std::vector<std::string> data;
	unsigned int line = 0;
	unsigned int col = 0;

	// TODO(fkp): Make this a timer
	unsigned int pointFlashFrameCounter = 0;

public:
	Buffer(BufferType type);

	// Movement of the point
	void movePointLeft();
	void movePointRight();
	void movePointUp();
	void movePointDown();
	void movePointHome();
	void movePointEnd();
	
	// Manipulations at the point
	void insertChar(char character);
	void backspaceChar();
	void deleteChar();
	void newLine();
};

#endif
