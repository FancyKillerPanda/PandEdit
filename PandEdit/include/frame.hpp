//  ===== Date Created: 15 April, 2020 ===== 

#if !defined(FRAME_HPP)
#define FRAME_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "buffer.hpp"
#include "point.hpp"

constexpr unsigned int FRAME_BORDER_WIDTH = 5;

struct Vector4f
{
	float x;
	float y;
	float width;
	float height;
};

class Font;

class Frame
{
public:
	inline static Frame* currentFrame = nullptr;
	inline static Frame* previousFrame = nullptr;
	inline static Frame* minibufferFrame = nullptr;
	inline static std::vector<Frame>* allFrames = nullptr;

	inline static std::vector<std::string> killRing;
	inline static int killRingPointer = -1; // -1 when nothing in the kill ring
	inline static DWORD lastClipboardSequenceNumber = 0;
	
	std::string name;

	// The real dimensions of the frame, as a percentage of the window
	// size (without the minibuffer).	
	// NOTE(fkp): The minibuffer will have a y-value of 1.0f and a
	// height of 0.0f, to indicate it should use the current font size
	// as the height
	Vector4f pcDimensions;
	unsigned int windowWidth;
	unsigned int windowHeight;
	
	Buffer* currentBuffer = nullptr;
	Point point;
	Point mark;	
	unsigned int lineTop = 0;

	// TODO(fkp): Make this a timer
	unsigned int pointFlashFrameCounter = 0;

private:
	inline static std::unordered_map<std::string, Frame*> framesMap;

public:
	Frame(std::string name, Vector4f dimensions, unsigned int windowWidth, unsigned int windowHeight, Buffer* buffer = nullptr, bool isActive = false);
	Frame(std::string name, Vector4f dimensions, unsigned int windowWidth, unsigned int windowHeight, BufferType type, std::string bufferName, bool isActive = false);
	~Frame();
	Frame(const Frame&) = delete;
	Frame& operator=(const Frame&) = delete;
	Frame(Frame&& other);
	Frame& operator=(Frame&& other);
	
	static Frame* get(const std::string& name);
	
	void makeActive();
	void switchToBuffer(Buffer* buffer);
	void updateWindowSize(unsigned int newWidth, unsigned int newHeight);
	Frame* splitVertically();
	Frame* splitHorizontally();
	unsigned int getNumberOfLines(Font* currentFont);

	std::pair<Point, Point> getPointStartAndEnd();
	std::string getTextPointToMark();
	void deleteTextPointToMark(bool appendToKillRing = true);

	// This is stuff that is common to all point manipulations
	void doCommonPointManipulationTasks();
	
	// Manipulations at the point
	void insertChar(char character);
	void backspaceChar(unsigned int num = 1);
	void deleteChar(unsigned int num = 1);
	void newLine();
	void insertString(const std::string& string);

	// Movement of the point
	void movePointLeft(unsigned int num = 1);
	void movePointRight(unsigned int num = 1);
	// TODO(fkp): Maybe store the number of lines once?
	void movePointUp(Font* currentFont);
	void movePointDown(Font* currentFont);
	void movePointHome();
	void movePointEnd();

	void moveView(int numberOfLines, bool movePoint);
	void recenterBufferAroundPoint(Font* currentFont);
	
	// Utility
	// TODO(fkp): Currently only registers space, do other word boundaries
	unsigned int findWordBoundaryLeft();
	unsigned int findWordBoundaryRight();
	void moveColToTarget();
	void adjustOtherFramePointLocations(bool insertion, bool lineWrap);

	// Copy/cut/paste
	void copyRegion();
	void paste();
	void pasteClipboard();
	void pastePop();

private:
	void init(std::string name, Vector4f dimensions, unsigned int windowWidth, unsigned int windowHeight, Buffer* buffer = nullptr, bool isActive = false);
};

#endif
