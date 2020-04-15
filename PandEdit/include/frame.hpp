//  ===== Date Created: 15 April, 2020 ===== 

#if !defined(FRAME_HPP)
#define FRAME_HPP

#include <string>
#include <unordered_map>

class Frame
{
public:
	static Frame* currentFrame;
	static Frame* previousFrame;

	std::string name;
	
	int x;
	int y;
	unsigned int width;
	unsigned int height;

private:
	static std::unordered_map<std::string, Frame*> framesMap;

public:
	Frame(std::string name, int x, int y, unsigned int width, unsigned int height, bool isActive = false);
	~Frame();
	static Frame* get(const std::string& name);

	void makeActive();
};

#endif
