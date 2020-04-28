//  ===== Date Created: 15 April, 2020 ===== 

#if !defined(FRAME_HPP)
#define FRAME_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "buffer.hpp"

constexpr unsigned int FRAME_BORDER_WIDTH = 5;

struct Vector4f
{
	float x;
	float y;
	float width;
	float height;
};

class Frame
{
public:
	static Frame* currentFrame;
	static Frame* previousFrame;
	static Frame* minibufferFrame;
	static std::vector<Frame>* allFrames;

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
	unsigned int line = 0;
	unsigned int col = 0;
	unsigned int targetCol = 0;
	
	unsigned int markLine = 0;
	unsigned int markCol = 0;

private:
	static std::unordered_map<std::string, Frame*> framesMap;

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

	// TODO(fkp): This should have a better return type
	// TODO(fkp): Maybe overload comparison operators in a point struct?
	std::pair<std::pair<int, int>, std::pair<int, int>> getPointStartAndEnd();
	std::string getTextPointToMark();
	void deleteTextPointToMark();
	
	void adjustOtherFramePointLocations(bool insertion, bool lineWrap);

private:
	void init(std::string name, Vector4f dimensions, unsigned int windowWidth, unsigned int windowHeight, Buffer* buffer = nullptr, bool isActive = false);
};

#endif
