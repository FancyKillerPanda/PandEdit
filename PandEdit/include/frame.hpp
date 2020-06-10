//  ===== Date Created: 15 April, 2020 ===== 

#if !defined(FRAME_HPP)
#define FRAME_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "buffer.hpp"
#include "point.hpp"
#include "timer.hpp"

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
	inline static std::vector<Frame*>* allFrames = nullptr;

	inline static std::vector<std::string> killRing;
	inline static int killRingPointer = -1; // -1 when nothing in the kill ring
	inline static DWORD lastClipboardSequenceNumber = 0;
	
	// NOTE(fkp): Parent is only valid if this is not the toplevel
	// frame. Children are only valid if this is not the bottommost
	// level of frame.
	Frame* parent = nullptr;
	Frame* childOne = nullptr;
	Frame* childTwo = nullptr;
	
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

	int currentTopLine = 0;
	int targetTopLine = 0;
	unsigned int numberOfLinesInView = 0;

	Timer pointFlashTimer;

	std::vector<std::string> popupLines;
	
	bool overwriteMode = false;

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
	void destroyBuffer(Buffer* buffer = nullptr);
	void updateWindowSize(unsigned int newWidth, unsigned int newHeight, Font* font);
	void split(bool vertically, Font* currentFont);
	void destroy();
	void getNumberOfLines(Font* currentFont);

	std::pair<Point, Point> getPointStartAndEnd();
	std::string getTextPointToMark();
	void deleteTextPointToMark(bool appendToKillRing = true);
	void deleteRestOfLine();

	// This is stuff that is common to all point/buffer manipulations
	void doCommonPointManipulationTasks();
	void doCommonBufferManipulationTasks();
	
	// Manipulations at the point
	void insertChar(char character);
	void backspaceChar(unsigned int num = 1, bool copyText = false);
	void deleteChar(unsigned int num = 1, bool copyText = false);
	void newLine();
	void insertString(const std::string& string);

	// Movement of the point
	void movePointLeft(unsigned int num = 1);
	void movePointRight(unsigned int num = 1);
	void movePointUp();
	void movePointDown();
	void movePointHome();
	void movePointEnd();
	void movePointToBufferStart();
	void movePointToBufferEnd();

	void moveView(int numberOfLines, bool movePoint);
	void centerPoint();

	void getRect(Font* currentFont, int* realPixelX, unsigned int* realPixelWidth, int* pixelX, int* pixelY, unsigned int* pixelWidth, unsigned int* pixelHeight);
	void getPointRect(Font* currentFont, unsigned int tabWidth, int framePixelX, int framePixelY, float* pointX, float* pointY, float* pointWidth, float* pointHeight);	
	Token* getTokenUnderPoint(bool includeEnd = false);
	
	// Utility
	unsigned int findWordBoundaryLeft();
	unsigned int findWordBoundaryRight();
	void moveColToTarget();
	void adjustOtherFramePointLocations(bool insertion, bool lineWrap);
	
	void updatePopups();
	void completeSuggestion();

	// Copy/cut/paste
	void copyRegion(std::string text = "");
	void paste();
	void pasteClipboard();
	void pastePop();

private:
	void init(std::string name, Vector4f dimensions, unsigned int windowWidth, unsigned int windowHeight, Buffer* buffer = nullptr, bool isActive = false);

	void deleteChildFrames(Frame* otherChild); // Called from one sibling
	void resizeChildrenToFitSize();
};

#endif
